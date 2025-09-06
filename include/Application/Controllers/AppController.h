#pragma once
#include <qobject.h>
#include "View/MainWindow.h"
#include "Infrastructure/Http/ApiClient.h"
#include "Enums/RouteEnums.h"

namespace Application::Controllers {
class AppController : public QObject {
    Q_OBJECT
  public:
    AppController(App::Views::MainWindow* main,
                  Invoke::Domain::Accounts::IAccountManager& accountManager,
                  Infrastructure::Http::ApiClient* api,
                  QObject* parent = nullptr);

  private:
    App::Views::MainWindow* main_;
    Invoke::Domain::Accounts::IAccountManager& accountManager_;
    Infrastructure::Http::ApiClient* api_;
  public slots:
    void page_navigation(Page page);
};
} // namespace Application::Controllers