#include <QApplication>
#include "ui/MainWindow.h"

/**
 * @brief Entry point of the Music Recognition application.
 *
 * This function initializes the Qt application, creates the main window,
 * and starts the event loop. The QApplication object manages
 * application-wide resources and the event dispatch system.
 */
int main(int argc, char *argv[]) {
    // Initialize the Qt application with command-line arguments
    QApplication app(argc, argv);

    // Create and display the main application window
    MainWindow w;
    w.show();

    // Enter Qt's event loop until the application exits
    return app.exec();
}