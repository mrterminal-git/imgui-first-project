#include "AppController.hpp"

void AppController::run() {
    // Create data to plot

    // Fetch data from the model
    dataManager.addSensor("sensor_1");
    dataManager.preloadData("sensor_1", 0.0, 100.0);
    auto data = dataManager.buffers_["sensor_1"].getData();

    // Render the view
    graphView.Draw("Data Viewer", data);
}

