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

The frontend connects to the backend at `http://127.0.0.1:1234` and does not start it automatically. Keep both the backend and MongoDB running while using account mode. The backend owns the account session, database access and account PDF export. Output folders are paths on the backend's computer, so run both executables on the same computer. The backend currently supports one active account session: use one frontend per backend, rather than treating it as a shared remote service.

Closing the frontend leaves the backend running; stop the backend separately with **Ctrl+C** in its terminal. Restarting the backend clears its active login session, so log in again from the frontend. Offline invoice operations and PDF export run entirely in the frontend without HTTP requests, a backend process or a database connection.

### Package the frontend and backend separately

After building both executables, install each component into its own directory. Use absolute install paths because Qt's deployment script requires them:

```powershell
cmake --install out/build/x64-release --prefix "$PWD/out/install/frontend" --component Frontend
cmake --install out/build/x64-release --prefix "$PWD/out/install/backend" --component Backend
```

Distribute the entire `out/install/frontend` folder, including its `bin` and `plugins` directories. Launch `bin/InvokeInvoiceSystem.exe`. This package contains the desktop UI, offline PDF support and HTTP client; it excludes the backend executable, MongoDB drivers and Qt HTTP server. The backend package has its own `bin/InvokeInvoiceBackend.exe` and dependencies. Neither package includes database credentials. On a computer without the Microsoft Visual C++ runtime, run the included `bin/vc_redist.x64.exe` installer first.

For account mode in this version, run the backend package on the same computer and set its MongoDB environment variables as above. The offline demo only needs the frontend package. For a `x64-relwithdebinfo` build, substitute that build directory in the install commands.

The executable split is the first step toward online hosting. Before deploying a shared backend, replace the single active account and invoice state with expiring user sessions authenticated on every request, enforce record ownership, configure the HTTPS API URL and hosting listener, and return PDF downloads or generate account PDFs in the frontend. The current local backend must not be exposed as a shared internet service.

### 5. Create your first account invoice

1. Sign up for an account, then log in.
2. Select **Configure business** on the dashboard and save your business details. You can include a website and a PNG or JPEG logo up to 5 MB.
3. Choose the business on the dashboard, then use **Create new client** to save a client for it. You can also add reusable stock with **Create stock item**.
4. Select **Create new invoice** and choose a saved client. Enter the invoice number, file name, output folder, creation date and due date. The available theme is **PEECE**.
5. Select **Next**, then choose **New item** or **Saved stock**. Enter a positive whole invoice quantity and a non-negative price, then select **Add item**. New items can be used just for this invoice, or saved with **Save this item to business stock** for reuse. Select **Finish invoice** once you have added all items.
6. Open the PDF in your chosen output folder. File-name spaces are replaced with underscores, and `.pdf` is appended if needed.

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

## Development and tests

For a build with debugging symbols, use `x64-relwithdebinfo` in both configure and build commands. Its output and dependency directories are under `out/build/x64-relwithdebinfo`; adjust the launch paths accordingly.

The following commands use the Release build and the runtime environment set above.

### UI, frontend and backend session tests

These three suites do not require MongoDB or a separately running backend. CTest configures Qt to run them without displaying windows.

- `BusinessSession` covers business selection, record browsing, client selection, reusable stock, optional record fields in both modes, and the offline demo. Offline checks drive the forms through PDF export, verify the PDF header and zero HTTP connections, check account isolation and export failures, and confirm that exiting clears the session.
- `FrontendSession` uses a local test HTTP server to check the frontend's login state and logout requests.
- `BackendServer` checks server startup, authentication handling, logout and the account session lifecycle.

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

```powershell
cmake --build --preset x64-release --target DatabaseTests
ctest --test-dir out/build/x64-release -R '^DatabaseHealth$' --output-on-failure
```

## Troubleshooting

| Problem | What to check |
| --- | --- |
| `cl`, `cmake` or `ninja` cannot be found | Use an x64 Developer PowerShell and check that the C++ and CMake build tools are installed. |
| CMake cannot find the vcpkg toolchain | Set `VCPKG_ROOT` to the bootstrapped vcpkg checkout before configuring. |
| Missing DLL or Qt platform plugin errors | Set `PATH` and `QT_PLUGIN_PATH` using the dependency directory for the build you are launching. |
| Login or signup cannot reach the database | Check that the backend and MongoDB are running, the backend's URI and credentials are correct, and database access is allowed. Restart the backend after changing environment variables. |
| The UI opens but local API requests fail | Start `InvokeInvoiceBackend.exe` separately on the same computer. Check its terminal for errors and ensure port `1234` is not already used by another process. |
| Clients or stock are missing | Select the correct business on the dashboard and refresh its records. Save a client for that business before creating an invoice. |
| The PDF is missing | Choose an existing, writable output folder and check the file name entered during invoice creation. Account PDFs are written by the backend; offline PDFs are written by the frontend. |
| Demo businesses, clients or stock disappeared | Demo records are cleared on **Exit demo** or when the app closes. Only exported PDFs persist. |

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
