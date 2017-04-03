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
	void on_spinBoxWorkers1_valueChanged(int value);
	void on_spinBoxWorkers2_valueChanged(int value);
	void on_spinBoxWorkers1Min_valueChanged(int value);
	void on_spinBoxWorkers1Max_valueChanged(int value);
	void on_spinBoxWorkers2Min_valueChanged(int value);
	void on_spinBoxWorkers2Max_valueChanged(int value);

	void replication_started(int replication);
	void replication_finished(int replication, replication_info info);
	void simulation_started(int workers1, int workers2);
	void simulation_finished(int workers1, int workers2, double wait_for_repair_total_time, double wait_in_queue_total_time, double queue_len_total, double time_in_service_total);
	void best_worker_count_found(int w1, int w2);
	void run_finished();
	void refresh_triggered(double time, refresh_info info);

private:
	void _prepare_plot();

private:
	Ui::MainWindow *_ui;
	sim_core_car_service *_sim;
	std::thread _thr;
};

#endif // MAINWINDOW_HPP
