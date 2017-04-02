#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "sim/sim_core_car_service.hpp"

#include <thread>
#include <QMainWindow>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_radioButtonCustomSeed_toggled(bool checked);
	void on_checkBoxRangeWorkers1_toggled(bool checked);
	void on_checkBoxRangeWorkers2_toggled(bool checked);
	void on_pushButtonStartSimulation_clicked();
	void on_pushButtonPauseResumeSimulation_clicked();
	void on_pushButtonStopSimulation_clicked();
	void on_checkBoxWatchModeEnabled_toggled(bool checked);
	void on_horizontalSliderSimSpeed_valueChanged(int value);
	void on_horizontalSliderSimRefreshRate_valueChanged(int value);

	void replication_finished(int replication, double wait_for_repair_time, double wait_in_queue_time);
	void simulation_finished(double wait_for_repair_total_time, double wait_in_queue_total_time);
	void refresh_triggered();

private:
	Ui::MainWindow *_ui;
	sim_core_car_service *_sim;
	std::thread _thr;
};

#endif // MAINWINDOW_HPP
