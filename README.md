# InvokeInvoiceSystem

A C++20 and Qt 6 desktop application for managing businesses, clients and stock, and generating PDF invoices. The desktop frontend and local backend run as separate executables on the same computer. Account records are stored in MongoDB; the offline demo keeps records in memory. Invoices are saved to a folder you choose.

## Run it on your machine

To try the app without an account, backend process or MongoDB, build and launch the frontend using steps 1, 2 and 4, then follow the [offline invoice demo](#try-the-offline-invoice-demo). Demo records exist only in memory and are discarded when you select **Exit demo** or close the frontend. Existing account records and account settings are unavailable in this mode; only the exported PDF is saved.

The included build presets target **64-bit Windows with MSVC and Ninja**. The instructions below build and run the application from source.

### 1. Install the prerequisites

- Git.
- Visual Studio 2022 or Build Tools for Visual Studio 2022, with **Desktop development with C++**, the Windows SDK and C++ CMake tools installed.
- CMake 3.24 or newer and Ninja, available in your terminal.
- vcpkg, set up as shown below, or an existing vcpkg checkout.
- For account mode, a running MongoDB server: either a local instance on port `27017` or a remote database you can access. The application needs permission to read and write its database. Offline mode does not require a database server.

Open an **x64 Developer PowerShell for Visual Studio** so that the MSVC compiler (`cl`) is available. Run the following commands in that terminal.

### 2. Get the source and set up vcpkg

```powershell
git clone https://github.com/FabianDim/InvokeInvoiceSystem.git
cd InvokeInvoiceSystem

# Skip cloning and bootstrapping if you already have vcpkg installed.
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
& C:\dev\vcpkg\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
```

If your vcpkg checkout is elsewhere, set `VCPKG_ROOT` to that directory. CMake uses the repository's `vcpkg.json` manifest and pinned registry baseline to install dependencies, including Qt; a separate Qt installation is not required. The first configure can take a while because it builds dependencies.

### 3. Configure MongoDB (account mode only)

For a local MongoDB instance, start the database service and set these variables in the terminal that will launch the backend:

```powershell
$env:MONGODB_URI = 'mongodb://localhost:27017/InvokeInvoiceSystem'
$env:MONGODB_DATABASE = 'InvokeInvoiceSystem'
```

For a remote or authenticated server, replace `MONGODB_URI` with its connection string. Use your database credentials and ensure the server allows connections from your machine. Percent-encode special characters in credentials embedded in the URI.

| Variable | Behavior |
| --- | --- |
| `MONGODB_URI` | Defaults to `mongodb://localhost:27017/InvokeInvoiceSystem` when unset or empty. |
| `MONGODB_DATABASE` | Overrides the database name in the URI. If neither specifies a name, the backend uses `InvokeInvoiceSystem`. |

These PowerShell settings apply to the current terminal and processes launched from it. Set them again in a new terminal, or configure them in your IDE's backend launch environment. The backend reads process environment variables; it does not load a `.env` file. The frontend does not need MongoDB credentials.

### 4. Build and launch

From the repository root:

```powershell
cmake --preset x64-release
cmake --build --preset x64-release --target InvokeInvoiceSystem InvokeInvoiceBackend
```

For the offline demo only, you can build just `--target InvokeInvoiceSystem`.

On Windows, vcpkg deploys runtime libraries beside the desktop executable, and
the build also runs `windeployqt` to deploy its plugins, including
`platforms/qwindows.dll`.
To repair missing plugins in an existing build, run
`cmake --build --preset x64-release --target deploy-qt` (or use the
`x64-relwithdebinfo` preset). `WindowsDesktopStartup` verifies startup with the
Windows platform plugin rather than the offscreen test plugin. Release ZIPs use
the separate packaging script to include the redistributable runtime as well.

For account mode, launch the backend in the terminal where you configured MongoDB:

```powershell
# Run from the repository root.
$deps = Join-Path $PWD 'out\build\x64-release\vcpkg_installed\x64-windows'
$env:PATH = "$deps\bin;$env:PATH"
$env:QT_PLUGIN_PATH = "$deps\Qt6\plugins"

& .\out\build\x64-release\InvokeInvoiceBackend.exe
```

Keep the backend terminal open while using account mode. Launch the frontend from the repository root, using a second terminal if the backend is running:

```powershell
# Make the vcpkg runtime libraries and Qt plugins available for this session.
$deps = Join-Path $PWD 'out\build\x64-release\vcpkg_installed\x64-windows'
$env:PATH = "$deps\bin;$env:PATH"
$env:QT_PLUGIN_PATH = "$deps\Qt6\plugins"

& .\out\build\x64-release\InvokeInvoiceSystem.exe
```

The desktop app connects to `https://invoice-backend-app-69d26786220b.herokuapp.com`
by default. MongoDB credentials belong in the Heroku backend's config vars;
desktop sign-in and business requests use HTTPS and do not query MongoDB locally.
The backend must include the session-token login API described below.

To use the embedded local server instead, set
`$env:INVOICE_API_URL = 'http://127.0.0.1:1234'` before launching the desktop app.
In that mode, configure MongoDB locally and leave `PORT` unset; port 1234 must
be free. Remove `INVOICE_API_URL` to restore the Heroku default. Other backend
URLs must use HTTPS (HTTP is accepted only for `127.0.0.1`).
Offline invoice operations run in memory without HTTP requests or a database connection.

Successful login returns an opaque, randomly generated token. The client sends
it as a bearer token on subsequent requests and revokes it on logout. Each token
has its own account identity and invoice state. Sessions expire after eight hours
and are held in memory on the single backend dyno, so restarting it requires
signing in again. The desktop keeps tokens only in memory; automatic sign-in
across application launches is not implemented.

When `PORT` is set (as it is on Heroku), the HTTP server listens on `0.0.0.0:$PORT` instead. The value must be an integer from 1 to 65535; an invalid or empty value logs an error and prevents the listener from starting. To restore the local default in PowerShell, run `Remove-Item Env:PORT -ErrorAction SilentlyContinue`.

### Build the backend Docker image

The root `Dockerfile` installs Ubuntu 24.04 build tools, Qt's Linux system dependencies,
and the libraries in `vcpkg.json`, using the repository's pinned vcpkg revision. It
compiles only `InvokeInvoiceBackend` and starts it as an unprivileged user. This
initial image retains the compiler and dependency installation; it is not a minimal
runtime image.

The `InvokeInvoiceBackend` CMake target runs a `QCoreApplication` event loop with
the existing HTTP routes, database services and PDF generator; it does not open
the desktop UI or require a display server. It exits with a nonzero status if
the HTTP listener cannot start.

To build and run it locally using the Windows prerequisites above:

```powershell
cmake --preset x64-release
cmake --build --preset x64-release --target InvokeInvoiceBackend
& .\out\build\x64-release\InvokeInvoiceBackend.exe
```

With `PORT` unset, visit `http://127.0.0.1:1234/` for the backend version number
(for example, `0.1.0`). The Heroku root URL returns the deployed version too.
Account routes require the MongoDB configuration described above.

With Docker configured to run Linux containers, build from the repository root:

```powershell
docker build --platform linux/amd64 -t invoke-invoice-backend .
```

The Docker build uses `triplets/x64-linux-release.cmake` for both target dependencies
and host tools. It sets `VCPKG_BUILD_TYPE=release`, skipping Debug dependency builds;
`CMAKE_BUILD_TYPE=Release` alone does not do this for vcpkg. See the
[vcpkg build-type documentation](https://learn.microsoft.com/en-us/vcpkg/users/triplets#vcpkg_build_type).

The first build still compiles Qt and can take considerable time and disk space.
Dependency layers are reused by the same Docker builder when the manifests,
triplets and preceding Dockerfile instructions are unchanged. Build parallelism
defaults to two jobs; change it with `--build-arg BUILD_JOBS=4` if your
builder has enough memory. `.dockerignore` excludes local build outputs and `.env`
files. Pass database credentials at runtime, never as Docker build arguments.

### Push the backend to Heroku

<<<<<<< Updated upstream
#### GitHub checks and frontend releases

Every push to `main` runs `.github/workflows/main-ci.yml`:

1. Choose a version for the CI builds and frontend release. With the current
   `0.1.0` base in `vcpkg.json`, workflow runs produce `0.1.1`, `0.1.2`, etc.
   The patch number is the base patch plus GitHub's workflow run number.
   Retries retain their version; failed or superseded runs can leave gaps.
2. Build the Dockerfile's `tests` stage and run every registered CTest entry,
   including PDF, database health, logging, business session, frontend session,
   backend server and test-discovery checks. Qt runs offscreen. Database checks
   use an authenticated, temporary MongoDB service, never the deployed database.
3. Only after all tests pass, build the Windows x64 frontend, bundle its DLLs,
   Qt plugins and MSVC runtime, and check the packaged executable's version
   without developer dependency paths.
4. Check that the backend's `runtime` Docker image builds successfully.
5. Publish a GitHub Release tagged `v<version>` at the tested commit, with the
   Windows ZIP and a SHA-256 checksum. Failed tests or builds prevent publication.

GitHub Actions does not push to Heroku's registry or release the backend. No
Heroku secrets or production database credentials are needed in GitHub.

Test reports are uploaded as the `test-results` artifact. Dependency layers are
cached between runs; the first build compiles Qt and can take considerably longer.
Release workflows are serialized to prevent overlapping publications.

Download the frontend from the repository's
[Releases page](https://github.com/FabianDim/InvokeInvoiceSystem/releases).
Extract the entire `InvokeInvoiceSystem-<version>-windows-x64.zip` and run
`InvokeInvoiceSystem.exe`; keep its DLLs and plugin folders together.
The executable supports `--version`, and the package includes `VERSION.txt`.
The GitHub release job uses the built-in `GITHUB_TOKEN` with `contents: write`;
no separate GitHub publishing token is needed. A draft is published only after
its assets upload successfully; reruns leave an already published release intact.

Local and Heroku source builds default to the version in `vcpkg.json` (`0.1.0`
currently). The workflow's incrementing frontend version is a CI build override;
it does not rewrite the repository version. Override a local build with
`cmake --preset x64-release -DINVOICE_APP_VERSION="0.1.42"` or
`docker build --build-arg APP_VERSION=0.1.42 -t invoke-invoice-backend .`.
Keep this workflow's identity and run counter when maintaining the release
pipeline; tag collisions with another commit fail instead of replacing a release.

The Python Object ID migration suite is currently unavailable because
`scripts/migrate_object_ids.py` is absent; CMake does not register it. Restoring
that suite also requires its Python dependencies and a local MongoDB replica set
for transactional integration tests. `InvoiceMenuTests.cpp` contains only a
commented-out test, so it has no executable test cases. All six active C++ suites
run in this workflow.

#### Heroku-managed automatic deployment

In the Heroku app's **Deploy** tab, connect this GitHub repository, select `main`,
check **Wait for GitHub checks to pass before deploy**, and select **Enable
Automatic Deploys**. Heroku then handles deployment after the checks succeed.
The repository's `heroku.yml` tells Heroku to build `Dockerfile` for the `web`
process; the final `runtime` stage supplies its startup command. The app must
use the `container` stack and have its MongoDB runtime config vars configured.
These dashboard settings must be enabled in Heroku; adding the workflow does
not change them. See [Heroku GitHub integration](https://devcenter.heroku.com/articles/github-integration)
and [Docker builds with heroku.yml](https://devcenter.heroku.com/articles/build-docker-images-heroku-yml).

#### Manual deployment
=======
If a remote build ends with `Build Timeout Expired` while compiling Qt, build on
your local Docker builder and upload the completed image using the command below.
Start Docker Desktop with Linux containers enabled first. This keeps compilation
outside Heroku's remote build deadline; a cold Qt build can exceed that deadline
even with Release-only dependencies. The timeout alone does not indicate a Qt
compiler error. Use the same local builder on subsequent builds to reuse its cache.
>>>>>>> Stashed changes

Heroku's registry requires Docker Image Manifest V2, Schema 2. With Docker's
containerd image store, `heroku container:push` can produce an OCI manifest that
the registry rejects with `unsupported`, even after uploading the layers.
Use Buildx's registry exporter to explicitly select Docker media types:

```powershell
heroku container:login
docker buildx build --platform linux/amd64 --provenance=false --sbom=false `
    --output "type=registry,name=registry.heroku.com/invoice-backend-app/web,oci-mediatypes=false" .
```

This builds and uploads the image. After a successful upload, release it:

```powershell
heroku container:release web --app invoice-backend-app
heroku logs --tail --app invoice-backend-app
```

The registry exporter avoids a separate `docker push` and reuses existing build
cache. See the [Heroku manifest requirements](https://devcenter.heroku.com/articles/container-registry-and-runtime#known-issues-and-limitations)
and [Docker exporter options](https://docs.docker.com/build/exporters/image-registry/).

The image supplies the backend executable and supports Heroku's `PORT`. Sessions
and invoice state are isolated by token. Finishing an invoice generates a PDF in
a temporary backend folder and returns it over HTTPS. The desktop saves it to
the folder selected in the invoice form and shows the saved path. Local paths
are never sent to the backend. Failed exports show an error and can be retried;
an unsuccessful download does not replace an existing PDF. The offline desktop
workflow continues to generate and save PDFs locally. Exported invoices are not
archived in MongoDB; keep the downloaded PDF for your records.

### 5. Create your first account invoice

1. Sign up for an account, then log in.
2. Select **Configure business** on the dashboard and save your business details. You can include a website and a PNG or JPEG logo up to 5 MB.
3. Choose the business on the dashboard, then use **Create new client** to save a client for it. You can also add reusable stock with **Create stock item**.
4. Select **Create new invoice** and choose a saved client. Enter the invoice number, file name, output folder, creation date and due date. The available theme is **PEECE**.
5. Select **Next**, then choose **New item** or **Saved stock**. Enter a positive whole invoice quantity and a non-negative price, then select **Add item**. New items can be used just for this invoice, or saved with **Save this item to business stock** for reuse. Select **Finish invoice** once you have added all items.
6. Open the PDF in your chosen output folder. File-name spaces are replaced with underscores, and `.pdf` is appended if needed.

## Debug logs

Debug logging is enabled by default in desktop and backend builds, including
Release builds. Events cover startup/shutdown, navigation and form validation,
sign-in/signup/logout and session expiry, HTTP requests, business selection and
cache invalidation, database reads/writes, offline records, PDF rendering,
download validation and local file writes. Warnings identify rejected operations
and failures; operation logs include elapsed milliseconds. `operation.returned`
means a function returned normally, not necessarily that validation succeeded.

The Windows desktop writes to
`%LOCALAPPDATA%\Invoke\InvokeInvoiceSystem\logs\application.log` as well as stderr.
It rotates at 5 MiB and keeps one previous file, `application.log.1`. To watch it:

```powershell
Get-Content "$env:LOCALAPPDATA\Invoke\InvokeInvoiceSystem\logs\application.log" -Tail 100 -Wait
```

The backend writes to stderr, which Heroku collects. After deploying the updated
backend, use `heroku logs --tail --app invoice-backend-app`. Each HTTP request has
an `X-Request-ID`; search for the matching `request_id` in client and server logs.
Server database and PDF logs inherit that request ID. Request logs include the
route, numeric method/operation, status, network error code and duration. They
describe response creation/receipt, not proof that a remote user opened the PDF.

Set `INVOICE_LOG_FILE` before launch to use another file (also supported by the
backend). If file writing or rotation fails, logging continues on stderr.
Use different files for separate running instances. Qt category rules control
verbosity without rebuilding:

```powershell
# Turn off debug noise while retaining info, warnings and errors.
$env:QT_LOGGING_RULES = 'invoice.*.debug=false'
# Or focus debug output on HTTP and PDF processing.
$env:QT_LOGGING_RULES = 'invoice.*.debug=false;invoice.http.debug=true;invoice.pdf.debug=true'
```

Categories are `invoice.app`, `invoice.ui`, `invoice.http`, `invoice.auth`,
`invoice.database`, `invoice.pdf` and `invoice.offline`. Remove `QT_LOGGING_RULES`
to restore the defaults. Logs omit credentials, tokens, request/response bodies,
customer details and invoice item contents. Database exceptions use numeric
driver codes instead of raw exception text. Request IDs are validated before
logging, and file log entries escape embedded newlines.

The `Logging` CTest checks HTTP status/body preservation, request correlation,
payload exclusion, exception logging, category filtering and file rotation.

## Try the offline invoice demo

This walkthrough also serves as a manual smoke test. No account, backend process or MongoDB server is needed; the frontend still requires the build dependencies above.

1. On the login or landing screen, select **Create an offline invoice**. Check that the offline banner and **Exit demo** button appear.
2. Enter `Demo business` as the business name and select **Use in demo**. Other business fields can remain blank. The dashboard selects the new business automatically.
3. Select **Create new client**, enter `Demo client` as the name and select **Use in demo**. Other client fields can remain blank.
4. Select **Create new invoice**, choose `Demo client`, enter invoice number `DEMO-001` and file name `demo invoice`, and choose an existing, writable output folder. Review the creation and due dates, keep the **PEECE** theme, then select **Next**.
5. Keep **New item** selected, enter `Demo service`, invoice quantity `2` and price each `15.50`, then select **Add item**. Reusable stock is optional: select **Keep this item in demo stock** before adding it to make it available under **Demo stock** during this session. You can also add stock through **Create stock item** on the dashboard.
6. Select **Finish invoice**. Check for the **PDF saved** message and open `demo_invoice.pdf` in the chosen folder. Verify the business, client, invoice number, dates and item; the two units at `15.50` should total `31.00`.
7. Select **Exit demo**, then enter the demo again. Check that the previous business, client and invoice items are gone. The exported PDF remains on disk.

The demo starts empty each time. Keeping an item in demo stock makes it reusable only within the current demo session; it does not save it to an account. A failed PDF export displays an error and allows you to retry.

## Features

In both account and offline mode, only the business name, client name, and stock item name and price are required when creating records. ABN/ACN, contact details, address, website, logo and stock metadata are optional. Blank stock-on-hand and margin default to zero; blank units default to `each`. Invoice line items still need a positive whole quantity and a non-negative price.

- Offline invoice demo with temporary businesses, clients and stock, plus local PDF export.
- Account registration and login with bcrypt password hashing.
- Multiple businesses per account, with clients and stock scoped to the selected business.
- Business record browsing and reusable stock selection during invoice creation.
- Business logos and website links on generated invoices.
- PDF invoices with client details, line items, calculated totals and support for multiple pages.
- MongoDB storage for accounts, businesses, clients and stock, with local PDF output.

## Project structure

| Location | Purpose |
| --- | --- |
| `src/View/UICode/Views/` | Qt screens for login, the dashboard, business records and invoice creation. |
| `src/View/MainWindow.cpp` | Main window and navigation. |
| `src/View/InvokeInvoiceSystem.cpp` | Frontend entry point; starts the desktop UI. |
| `src/Backend/InvokeInvoiceBackend.cpp` | Backend entry point; starts the local HTTP server for account mode. |
| `src/Application/` | Controllers, account handling and business workflows. |
| `src/Application/Invoices/OfflineInvoiceSession.cpp` | In-memory demo records, invoice validation and local PDF export. |
| `src/Domain/` | Account, business, client, stock and invoice models. |
| `src/Infrastructure/` | MongoDB access, HTTP client/server, password hashing and PDF generation. |
| `include/` | Headers corresponding to the application layers. |
| `forms/` | Qt forms, stylesheets and bundled resources. |
| `tests/` | Qt-based automated tests. |
| `CMakeLists.txt` / `CMakePresets.json` | Build targets and Windows build presets. |
| `vcpkg.json` / `vcpkg-configuration.json` | Dependency manifest and registry configuration. |

## Saving records and business invoice settings

Save on a business, client or stock form returns to the dashboard after the save
succeeds. **Save and add new business/client/stock item** keeps the form open for
the next record. Failed saves retain the entered details. Country fields use a
list; stock margins are entered and shown as percentages (25 means 25%). Stored
margin values remain fractional for compatibility (0.25).

Select a business on the dashboard, then open **Business invoice settings**.
The invoice prefix is saved on that business in MongoDB and follows it across
sign-ins and computers. It defaults to `INV-`; save an empty prefix for numbers
only. The default PDF folder is saved per business on this computer and survives
app restarts. Click **Save business invoice settings** to save both defaults.

Invoice numbers are assigned automatically when you continue to invoice items,
using a persistent, atomic counter per business: `INV-000001`, `INV-000002`, etc.
Changing the prefix does not reset the counter. Abandoned invoice drafts can leave
gaps. The assigned number is also the default PDF filename if no name was entered.

Finishing an invoice deducts its saved stock quantities and stores an invoice
record in one MongoDB transaction. Insufficient stock or an item from another
business rejects the whole operation without partial deductions. Re-saving the
same completed invoice PDF does not deduct stock again. New invoice-only items
do not affect stock. Account invoice completion requires MongoDB Atlas or a
replica set; standalone MongoDB cannot perform these transactions. Offline demo
invoices apply the same inventory rules to their temporary records.

Business records show names and details. Right-click a row to view or copy its
record ID.

To run the HTTP/database regression tests, start an isolated local MongoDB replica
set, install `pymongo` in your test Python environment, then run:

```powershell
$env:INVOICE_TEST_MONGODB_URI = 'mongodb://127.0.0.1:27028/?directConnection=true'
$env:INVOICE_TEST_BACKEND = (Resolve-Path './out/build/x64-release/InvokeInvoiceBackend.exe').Path
python -m unittest discover -s tests -p test_invoice_workflow.py -v
```

The suite creates and removes its own test databases. It verifies prefix and
counter persistence after backend restart, blank prefixes, concurrent numbering,
stock deductions, repeated exports, transaction rollback and business isolation.
It can also be registered with CTest using `-DINVOICE_ENABLE_WORKFLOW_TESTS=ON`
and a Python interpreter with `pymongo` installed; the MongoDB test URI must still
be supplied explicitly.

## Development and tests

### MongoDB identities and upgrading existing data

All persisted users, businesses, clients, stock and archived invoice records use
MongoDB's native `_id` ObjectId. Inserts omit `_id` and use the driver's returned
`inserted_id`; the application no longer generates `USR`, `BUS`, `CLI`, `STK` or
`INV` record IDs. The editable invoice number remains a separate business field.

MongoDB relationships (`UserID`, `BusinessID`, `ClientID`, `StockID` and reference
arrays) contain BSON ObjectIds. JSON records expose their primary identity as
`"_id": "<24 hexadecimal characters>"`. Foreign-key fields in API requests and
response envelopes keep their existing names and carry ObjectId hex strings.
Login returns `user._id`; resource creation returns `_id`. Deploy the matching
backend and desktop versions together. Existing sessions must sign in again.
Business reads and child writes check the authenticated owner; knowing an
ObjectId does not grant access.

The offline demo has no MongoDB records and uses a session-local `DEMO-<number>`
counter with the same JSON `_id` field. UUIDs used for HTTP request correlation
are unrelated to record identity and remain in place.

Before running this version against existing data, stop application writers and
take a database backup. The migration preserves every existing `_id`, converts
references, removes redundant primary-ID fields, and leaves the unused legacy
`counters` collection untouched. It rejects ambiguous duplicate legacy IDs,
missing references, conflicting ownership, and non-ObjectId primary keys rather
than guessing which record is intended. Resolve those records before retrying.

```powershell
python -m pip install -r scripts/requirements.txt
# Use the intended database connection, as described in the setup instructions.
# MONGODB_URI must be explicitly set; the script has no default connection.
python scripts/migrate_object_ids.py --database InvokeInvoiceSystem

# After reviewing the dry-run counts, apply while application writers are stopped.
python scripts/migrate_object_ids.py --database InvokeInvoiceSystem --apply
```

`--apply` revalidates and commits all changes in one transaction. It requires
Atlas or a MongoDB replica set, including a single-node replica set for local
use. A standalone server can run the dry run, but cannot apply this migration.
The migration can be rerun safely; migrated records require no further changes.
Large databases whose conversion exceeds MongoDB's transaction limits need a
separate staged migration. A failed transaction leaves the original data intact.
Start the updated backend only after migration completes.

Run migration validation tests with:

```powershell
python -m unittest discover -s tests -p test_object_ids.py -v
```

To also test actual MongoDB inserts, BSON relationships, HTTP record identities,
cross-account rejection, invoice PDF export and transactional migration, provide
an isolated local MongoDB replica set and the built backend:

```powershell
$env:INVOICE_TEST_MONGODB_URI = 'mongodb://127.0.0.1:27017/?directConnection=true'
$env:INVOICE_TEST_BACKEND = (Resolve-Path out/build/x64-release/InvokeInvoiceBackend.exe).Path
python -m unittest discover -s tests -p test_object_ids.py -v
```

The integration tests accept only loopback MongoDB connections. They create and
remove their own uniquely named test databases and launch their own backend
processes; they do not use `MONGODB_URI` or the application's database. Make the
backend runtime DLLs available in `PATH` as described above.

For a build with debugging symbols, use `x64-relwithdebinfo` in both configure and build commands. Its output and dependency directories are under `out/build/x64-relwithdebinfo`; adjust the launch paths accordingly.

The following commands use the Release build and the runtime environment set above.

### Build and check all test runners

From an x64 Developer PowerShell for Visual Studio, configure and build all six
Qt Test executables, then verify that each runner starts and lists its tests:

```powershell
cmake --preset x64-release
cmake --build --preset x64-release --target all-tests
ctest --test-dir out/build/x64-release -L smoke --output-on-failure
```

The `smoke` checks use Qt Test's `-functions` option. They do not execute test
assertions, access MongoDB, or establish that the tests pass. CTest sets Qt's
offscreen platform, and Windows builds copy the required offscreen and JPEG
plugins beside the runners.

| CTest suite | Executable | Requirements |
| --- | --- | --- |
| `PDFGen` | `InvokeInvoiceSystemTests` | Local; PDFs use temporary directories. |
| `Logging` | `LoggingTests` | Local. |
| `BusinessSession` | `BusinessSessionTests` | Local; offscreen UI. |
| `FrontendSession` | `FrontendSessionTests` | Local test HTTP server. |
| `BackendServer` | `BackendServerTests` | Local HTTP server; no MongoDB. |
| `DatabaseHealth` | `DatabaseTests` | Live MongoDB credentials for actual test execution. |

Run the local test bodies, or list all registered checks:

```powershell
ctest --test-dir out/build/x64-release -L local --output-on-failure
ctest --test-dir out/build/x64-release -N
```

Running CTest without a filter also runs the database health suite against the
configured database.

`tests/InvoiceMenuTests.cpp` contains only a Google Test fixture and a commented
test; it has no active test cases. `tests/test_object_ids.py` depends on
`scripts/migrate_object_ids.py`. When that script is absent, CMake reports the
Python suite as unavailable and does not register it. When present, the suite
is registered as `ObjectIds` and requires Python plus `scripts/requirements.txt`.

### UI, frontend and backend session tests

These three suites do not require MongoDB or a separately running backend. CTest configures Qt to run them without displaying windows.

- `BusinessSession` covers business selection, record browsing, client selection, reusable stock, optional record fields in both modes, and the offline demo. Offline checks drive the forms through PDF export, verify the PDF header and zero HTTP connections, check account isolation and export failures, and confirm that exiting clears the session.
- `FrontendSession` uses a local test HTTP server to check the frontend's login state and logout requests.
- `BackendServer` checks constructor-based server startup, protected routes, malformed login requests, database-unavailable responses, bearer-token lifecycle, and local PDF generation. Successful database-backed login and account isolation are covered by the optional Python integration suite.

```powershell
cmake --build --preset x64-release --target BusinessSessionTests FrontendSessionTests BackendServerTests
ctest --test-dir out/build/x64-release -R '^(BusinessSession|FrontendSession|BackendServer)$' --output-on-failure
```

To run only the demo and optional-field checks after building `BusinessSessionTests`, use Qt Test's function filters:

```powershell
$env:QT_ASSUME_STDERR_HAS_CONSOLE = '1'
& .\out\build\x64-release\BusinessSessionTests.exe -platform offscreen `
    offline_invoice_reuses_forms_and_exports_without_network `
    offline_api_blocks_accounts_and_online_records_and_handles_pdf_failure `
    optional_record_fields
```

The demo tests export PDFs into temporary directories that are removed when the tests finish. The full workflow also writes `offline-business-ui.png`, `offline-dashboard-ui.png` and `offline-invoice-ui.png` to the test working directory (the build directory when run through CTest). Use the manual walkthrough above to inspect a retained PDF's layout and totals; the automated demo checks verify file creation and the PDF header, not its rendered contents.

### Database health checks

These read-only integration tests check connectivity, authentication, writable-primary status and access to the `Users` collection. They require an explicitly set `MONGODB_URI` with valid database credentials. An unauthenticated local connection will fail the authentication check even if the app can use it.

On Windows, the manifest selects the MongoDB C driver's `openssl` feature to
avoid the pinned driver's native TLS handshake failure (`0x80090326`) with
Atlas. Certificate verification remains enabled. Reconfigure CMake after pulling
this dependency change so vcpkg rebuilds the MongoDB drivers before running tests.

```powershell
cmake --build --preset x64-release --target DatabaseTests
ctest --test-dir out/build/x64-release -R '^DatabaseHealth$' --output-on-failure
```

## Troubleshooting

| Problem | What to check |
| --- | --- |
| `cl`, `cmake` or `ninja` cannot be found | Use an x64 Developer PowerShell and check that the C++ and CMake build tools are installed. |
| CMake cannot find the vcpkg toolchain | Set `VCPKG_ROOT` to the bootstrapped vcpkg checkout before configuring. |
| Docker build ends with `Build Timeout Expired` while compiling Qt | Use the Release-only Docker triplet and the local Buildx upload workflow above. A cold Qt build may still exceed a remote builder's deadline. |
| Missing DLL or Qt platform plugin errors | Set `PATH` and `QT_PLUGIN_PATH` using the dependency directory for the build you are launching. |
| Login or signup cannot reach the database | Check that the backend and MongoDB are running, the backend's URI and credentials are correct, and database access is allowed. Restart the backend after changing environment variables. |
| The UI opens but local API requests fail | Start `InvokeInvoiceBackend.exe` separately on the same computer. Check its terminal for errors and ensure port `1234` is not already used by another process. |
| Clients or stock are missing | Select the correct business on the dashboard and refresh its records. Save a client for that business before creating an invoice. |
| The PDF is missing | Choose an existing, writable output folder and check the file name entered during invoice creation. Account PDFs are written by the backend; offline PDFs are written by the frontend. |
| Demo businesses, clients or stock disappeared | Demo records are cleared on **Exit demo** or when the app closes. Only exported PDFs persist. |

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
