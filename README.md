# InvokeInvoiceSystem

A C++20 and Qt 6 desktop application for managing businesses, clients and stock, and generating PDF invoices. Records are stored in MongoDB; invoices are saved to a folder you choose.

## Run it on your machine

The included build presets target **64-bit Windows with MSVC and Ninja**. The instructions below build and run the application from source.

### 1. Install the prerequisites

- Git.
- Visual Studio 2022 or Build Tools for Visual Studio 2022, with **Desktop development with C++**, the Windows SDK and C++ CMake tools installed.
- CMake 3.24 or newer and Ninja, available in your terminal.
- vcpkg, set up as shown below, or an existing vcpkg checkout.
- A running MongoDB server: either a local instance on port `27017` or a remote database you can access. The application needs permission to read and write its database.

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

### 3. Configure MongoDB

For a local MongoDB instance, start the database service and use:

```powershell
$env:MONGODB_URI = 'mongodb://localhost:27017/InvokeInvoiceSystem'
$env:MONGODB_DATABASE = 'InvokeInvoiceSystem'
```

For a remote or authenticated server, replace `MONGODB_URI` with its connection string. Use your database credentials and ensure the server allows connections from your machine. Percent-encode special characters in credentials embedded in the URI.

| Variable | Behavior |
| --- | --- |
| `MONGODB_URI` | Defaults to `mongodb://localhost:27017/InvokeInvoiceSystem` when unset or empty. |
| `MONGODB_DATABASE` | Overrides the database name in the URI. If neither specifies a name, the app uses `InvokeInvoiceSystem`. |

These PowerShell settings apply to the current terminal and processes launched from it. Set them again in a new terminal, or configure them in your IDE's launch environment. The application reads process environment variables; it does not load a `.env` file.

### 4. Build and launch

From the repository root:

```powershell
cmake --preset x64-release
cmake --build --preset x64-release --target InvokeInvoiceSystem

# Make the vcpkg runtime libraries and Qt plugins available for this session.
$deps = Join-Path $PWD 'out\build\x64-release\vcpkg_installed\x64-windows'
$env:PATH = "$deps\bin;$env:PATH"
$env:QT_PLUGIN_PATH = "$deps\Qt6\plugins"

& .\out\build\x64-release\InvokeInvoiceSystem.exe
```

Keep MongoDB running while using the app. The desktop executable starts its own HTTP server on `127.0.0.1:1234`; no separate backend process is needed. That port must be free, so run one app instance at a time.

### 5. Create your first invoice

1. Sign up for an account, then log in.
2. Select **Configure business** on the dashboard and save your business details. You can include a website and a PNG or JPEG logo up to 5 MB.
3. Choose the business on the dashboard, then use **Create new client** to save a client for it. You can also add reusable stock with **Create stock item**.
4. Select **Create new invoice** and choose a saved client. Enter the invoice number, file name, output folder, creation date and due date. The available theme is **PEECE**.
5. Select **Next**, add invoice items with their quantities and prices, then select **Finish invoice**.
6. Open the PDF in your chosen output folder. File-name spaces are replaced with underscores, and `.pdf` is appended if needed.

## Features

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
| `src/View/InvokeInvoiceSystem.cpp` | Application entry point; starts the desktop UI and embedded server. |
| `src/Application/` | Controllers, account handling and business workflows. |
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

### UI and session tests

These tests use a local test HTTP server and do not require MongoDB. CTest configures Qt to run them without displaying windows.

```powershell
cmake --build --preset x64-release --target BusinessSessionTests
ctest --test-dir out/build/x64-release -R '^BusinessSession$' --output-on-failure
```

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
| Login or signup cannot reach the database | Check that MongoDB is running, the URI and credentials are correct, and remote access is allowed. Restart the app after changing environment variables. |
| The UI opens but local API requests fail | Check that port `1234` is available and another app instance is not already running. |
| Clients or stock are missing | Select the correct business on the dashboard and refresh its records. Save a client for that business before creating an invoice. |
| The PDF is missing | Choose an existing, writable output folder and check the file name entered during invoice creation. |

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
