# Security review — 12 November 2026

Update: the subsequent ObjectId change addresses finding 3 in source: persisted
identities now come from MongoDB `_id`, references use BSON ObjectIds, and online
resource queries check ownership. Existing databases need the migration documented
in README.md before deployment. The build configuration blockers noted below
were also repaired. The original findings below describe the pre-change snapshot;
the other security findings are not claimed fixed by the identity change.

Scope: the current local source for HTTP authentication, account/resource ownership, PDF inputs, password hashing and session cleanup. Findings describe this checkout; deployed code and live MongoDB indexes were not inspected. No requests were sent to the hosted backend and no database records were read or changed.

## 1. High: a business logo can read an image from the backend filesystem

**Status: fixed in local source (13 November 2026); deployment unverified.**
Business creation no longer persists client-supplied `LogoPath`. Business queries
and both database-to-business loading paths ignore legacy stored paths. The PDF
renderer uses only `LogoData` bytes and omits the logo when those bytes are absent.
Existing database documents do not need path removal for this fix to take effect.

**Fix validation:** rebuilt the backend, desktop app, and affected test executables
in Release and RelWithDebInfo. The complete `BackendServer` and `PDFGen` CTest
suites pass in both configurations. The logo regression supplies a real local PNG
with empty and omitted `LogoData` and asserts that no image is embedded. A positive
PDF test verifies that valid `LogoData` still embeds an image. Database write/read
filtering was checked in source; no live database or hosted deployment was tested.
The evidence and original validation below describe the pre-fix snapshot.

**Evidence:** `src/Infrastructure/Database/Mongo/MongoDBDataManager.cpp:371` stores the request's `LogoPath`. `src/Application/Business/SetBusinessFromDB.cpp:49` restores it; `src/Infrastructure/Database/Services/InvoiceServices.cpp:53` passes it to the invoice business. `src/Infrastructure/Pdf/InvoicePdfGenerator.cpp:313–317` falls back to `HPDF_LoadPngImageFromFile` when `LogoData` is empty. The resulting PDF is returned by `src/Infrastructure/Http/FakeServer.cpp:94`.

An authenticated account can create its own business with an empty/omitted `LogoData` and a `LogoPath` naming a backend-readable PNG, then export an invoice for that business. The file's image content can be embedded in the downloaded PDF. No file upload or access to another account is needed. The file must exist, be readable, and be accepted by the PNG loader: this is not evidence that arbitrary text files or environment variables can be downloaded.

**Remediation:** discard client-supplied filesystem paths in online business creation and in the backend invoice boundary, including paths already persisted in MongoDB. Use validated image bytes for online rendering. Keep any desktop/offline path support explicitly separate. Add a regression that seeds a local PNG and verifies that an online invoice cannot load it through `LogoPath`.

**Validation:** complete source data-flow trace; no live HTTP/database reproduction.

## 2. High: one account can exhaust the global login capacity

**Evidence:** `include/Infrastructure/Http/AuthSessions.h:23–49` allows repeated sessions for the same identity until the global count reaches 1,000; each lasts eight hours. `src/Infrastructure/Http/FakeServer.cpp:237–244` creates another session after every successful password check and returns HTTP 503 when capacity is exhausted.

A user who repeatedly logs in without revoking tokens can consume every slot and prevent unrelated accounts from signing in. Existing sessions remain valid. In the absence of revocation or a backend restart, capacity returns as tokens expire; an attacker can continue replenishing it.

**Remediation:** enforce a small per-account active-session limit, decide how replacement/revocation should work, and rate-limit login issuance. Retain a global safety limit without allowing one account to monopolize it.

**Validation:** compiled the actual `AuthSessions` header into an isolated local probe. One synthetic identity received all 1,000 slots; another identity was rejected immediately and at a simulated seven hours, then accepted at eight hours. No HTTP traffic or database was involved.

## 3. High, conditional on database constraints: 32-bit IDs can break tenant isolation

**Evidence:** `src/Infrastructure/Database/Mongo/MongoDBDataManager.cpp:169–170` creates user IDs from eight hexadecimal UUID characters. The same truncation creates business/client/stock IDs at lines 350–353. Account reads resolve `Users` by `UserID` alone (lines 204 and 248); business and child reads resolve by their truncated IDs without also verifying the stored parent/owner (lines 262, 298 and 308). Child writes likewise load the business by `BusinessID` alone (line 393).

Different documents can receive the same 32-bit application ID. If duplicates are permitted, authentication identifies one account by email but subsequent authorization can resolve a different account with the same `UserID`. Business/client/stock collisions similarly make ownership references ambiguous. About 77,000 independently generated IDs in one namespace produce approximately a 50% chance of at least one collision; this is a birthday-collision estimate, not the probability of hitting a chosen victim.

No application code creating unique indexes was found. Live indexes may already reject duplicates, which would change the immediate impact to failed creation; that deployment condition remains unverified. MongoDB's default unique `_id` index does not enforce uniqueness of these separate fields. See [MongoDB indexes](https://www.mongodb.com/docs/manual/indexes/) and [unique indexes](https://www.mongodb.com/docs/manual/core/index-unique/create/).

**Remediation:** use full UUIDs or native ObjectIds, enforce uniqueness in the database, and include the authenticated owner/parent in resource queries. Audit existing duplicate IDs and references before migration; merely changing the generator does not repair ambiguous existing records.

**Validation:** source analysis; no live index inspection or collision injection.

## 4. Medium: the Windows bcrypt salt fallback repeats salts within a second

**Evidence:** `src/Infrastructure/Security/PasswordHashing/openbsd.h:15–25` uses `rand() % 256`, reseeded with `time(NULL)`. `src/Infrastructure/Security/PasswordHashing/bcrypt.cpp:321–329` initializes that generator for every hash. The existing Windows build caches report no system `arc4random_buf`, selecting this fallback.

Hashes generated in the same second receive identical salts. Salt predictability alone does not reveal passwords, but salt reuse defeats per-record separation of offline guessing work for those records and exposes equal-password relationships if hashes are disclosed. The branch using the operating system's `arc4random_buf` is not implicated by this finding; bearer-token generation uses a separate system generator.

**Remediation:** use an operating-system cryptographic random source on every supported platform and fail closed if it fails. Remove the time-seeded fallback. Existing affected password hashes require rehashing on successful authentication or a password reset.

**Validation:** compiled the repository's actual bcrypt sources on Windows. Two different synthetic passwords hashed within one clock second produced identical bcrypt salt prefixes. No real credentials were used or printed.

## 5. Medium: unthrottled authentication blocks the server event loop

**Evidence:** `src/Infrastructure/Http/FakeServer.cpp:205` and `:237` invoke synchronous database/password operations inside request callbacks. `src/Infrastructure/Database/Mongo/MongoDBDataManager.cpp:118–131` performs the database lookup and bcrypt verification inline; signup hashes inline at line 176. No application login/signup throttling was found. The backend runs these handlers through its single main Qt event loop (`src/Backend/main.cpp:24–30`).

Repeated authentication attempts can occupy the request-processing thread with bcrypt and database waits, delaying unrelated requests. Incorrect passwords for an existing account exercise bcrypt; open signup can also trigger hashing. Actual throughput and any external rate limiting were not measured. Qt documents explicit concurrent handling through `QFuture`/`QtConcurrent`: [QHttpServer documentation](https://doc.qt.io/qt-6.12/qhttpserver.html).

**Remediation:** throttle expensive authentication work by account and appropriately determined client address, enforce request/field limits, and move database/password work to a bounded worker queue. Use database clients suitable for that concurrency model. Avoid an unbounded queue or sharing the current MongoDB client unsafely across workers.

**Validation:** source analysis; no traffic flood or hosted-service load test.

## Verification limits and supporting artifacts

- The isolated probe built and exited successfully. Its source is `out/security-review/probe.cpp`; output is `out/security-review/probe-results.txt`.
- Probe output:

  ```text
  sessions_for_one_account=1000
  unrelated_account_rejected=1
  still_rejected_at_7_hours=1
  accepted_at_8_hours=1
  same_second_salts_identical=1
  ```

- Configuring the full project failed at `CMakeLists.txt:240`: `Flow control statements are not properly nested.` The final `endforeach()` has no matching loop. Full application tests were therefore not run against this checkout.
- `tests/BackendServerTests.cpp` still uses removed `Server::start`, `port`, and `error_string` APIs and expects a shared account-manager session. `tests/FrontendSessionTests.cpp` supplies a successful login response without the now-required token/user identity. These tests need updating before they can establish security coverage for the current protocol. Older executable files are not evidence that current source passes.
- A suspected late-login race was not retained as a finding: `ApiClient::clear_session()` disconnects and aborts outstanding replies before they can authenticate the new session.
- No production source was changed during this review. Existing concurrent edits to the login-page source/header were left untouched.
