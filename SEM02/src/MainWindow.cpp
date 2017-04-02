#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "sim/sim_core_car_service.hpp"

using time_converter_func = double (*)(const double&);
static const QStringList timeUnitItems = {"sekúnd", "minút", "hodín", "dní"};
static const QList<time_converter_func> funcs = {[](const double &time) { return time; }, minutes<double>, hours<double>, days<double>};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  _ui(new Ui::MainWindow), _sim(new sim_core_car_service)
{
	_ui->setupUi(this);
	_ui->comboBoxTimeUnit->addItems(timeUnitItems);
	_ui->comboBoxTimeUnit->setCurrentIndex(3);
	_ui->spinBoxCustomSeed->setMaximum(std::numeric_limits<int>::max());
	_ui->labelSimSpeedValue->setText(QString::number(_ui->horizontalSliderSimSpeed->value()));
	_ui->labelSimRefreshRateValue->setText(QString("%1 s").arg(_ui->horizontalSliderSimRefreshRate->value()));

	connect(_sim, SIGNAL(replication_finished(int,double,double)), this, SLOT(replication_finished(int,double,double)));
	connect(_sim, SIGNAL(simulation_finished(double,double)), this, SLOT(simulation_finished(double,double)));
	connect(_sim, SIGNAL(refresh_triggered()), this, SLOT(refresh_triggered()));
}

MainWindow::~MainWindow()
{
	if (_sim->get_state() == sim_core_car_service::state::RUNNING || _thr.joinable())
	{
		_sim->stop();
		_thr.join();
	}
	delete _sim;
	delete _ui;
}

void MainWindow::on_radioButtonCustomSeed_toggled(bool checked)
{
	_ui->spinBoxCustomSeed->setEnabled(checked);
}

void MainWindow::on_checkBoxRangeWorkers1_toggled(bool checked)
{
	_ui->spinBoxWorkers1->setEnabled(!checked);
	_ui->spinBoxWorkers1Min->setEnabled(checked);
	_ui->spinBoxWorkers1Max->setEnabled(checked);
}

void MainWindow::on_checkBoxRangeWorkers2_toggled(bool checked)
{
	_ui->spinBoxWorkers2->setEnabled(!checked);
	_ui->spinBoxWorkers2Min->setEnabled(checked);
	_ui->spinBoxWorkers2Max->setEnabled(checked);
}

void MainWindow::on_pushButtonStartSimulation_clicked()
{
	_ui->pushButtonStartSimulation->setEnabled(false);
	_ui->pushButtonPauseResumeSimulation->setEnabled(true);
	_ui->pushButtonStopSimulation->setEnabled(true);

	auto unitIndex = _ui->comboBoxTimeUnit->currentIndex();
	auto time = funcs[unitIndex](_ui->spinBoxReplicationDuration->value());
//	_ui->labelTimeValue->setText(sim_time_as_string(time));
	auto replications = _ui->spinBoxReplications->value();
	_thr = std::thread(&sim_core_car_service::simulate, _sim, replications, time);
}

void MainWindow::on_pushButtonPauseResumeSimulation_clicked()
{
	auto state = _sim->get_state();
	_sim->pause_resume();

	if (state == sim_core_car_service::state::RUNNING)
	{
		_ui->pushButtonPauseResumeSimulation->setText("Pokračovať");
	}
	else
	{
		_ui->pushButtonPauseResumeSimulation->setText("Pozastaviť");
	}
}

void MainWindow::on_pushButtonStopSimulation_clicked()
{
	_sim->stop();
	_thr.join();

	_ui->pushButtonStartSimulation->setEnabled(true);
	_ui->pushButtonPauseResumeSimulation->setEnabled(false);
	_ui->pushButtonPauseResumeSimulation->setText("Pozastaviť");
	_ui->pushButtonStopSimulation->setEnabled(false);
}

void MainWindow::on_checkBoxWatchModeEnabled_toggled(bool checked)
{
	_sim->set_watch_mode_active(checked);
	_ui->horizontalSliderSimSpeed->setEnabled(checked);
	_ui->horizontalSliderSimRefreshRate->setEnabled(checked);
}

void MainWindow::on_horizontalSliderSimSpeed_valueChanged(int value)
{
	_sim->set_sim_speed(_ui->horizontalSliderSimSpeed->maximum() - value);
	_ui->labelSimSpeedValue->setText(QString::number(value));
}

void MainWindow::on_horizontalSliderSimRefreshRate_valueChanged(int value)
{
	_sim->set_refresh_rate(value);
	_ui->labelSimRefreshRateValue->setText(QString("%1 s").arg(value));
}

void MainWindow::replication_finished(int replication, double wait_for_repair_time, double wait_in_queue_time)
{
	_ui->statusBar->showMessage(QString("Replikácia %1").arg(replication), 5000);
}

void MainWindow::simulation_finished(double wait_for_repair_total_time, double wait_in_queue_total_time)
{
	_thr.join();

	_ui->pushButtonStartSimulation->setEnabled(true);
	_ui->pushButtonPauseResumeSimulation->setEnabled(false);
	_ui->pushButtonPauseResumeSimulation->setText("Pozastaviť");
	_ui->pushButtonStopSimulation->setEnabled(false);
}

void MainWindow::refresh_triggered()
{
	_ui->labelTimeValue->setText(sim_time_as_string(_sim->get_cur_time()));
}
