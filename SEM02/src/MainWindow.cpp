#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#define PAUSE_TEXT     "Pozastaviť"
#define RESUME_TEXT    "Pokračovať"

using time_converter_func = double (*)(const double&);
static const QStringList timeUnitItems = {"sekúnd", "minút", "hodín", "dní"};
static const QList<time_converter_func> funcs = {seconds, minutes, hours, days};

Q_DECLARE_METATYPE(refresh_info)
Q_DECLARE_METATYPE(replication_info)

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  _ui(new Ui::MainWindow), _sim(new sim_core_car_service)
{
	qRegisterMetaType<refresh_info>();
	qRegisterMetaType<replication_info>();

	_ui->setupUi(this);
	_ui->labelUsedSeedValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
	_ui->comboBoxTimeUnit->addItems(timeUnitItems);
	_ui->comboBoxTimeUnit->setCurrentIndex(3);
	_ui->spinBoxCustomSeed->setMaximum(std::numeric_limits<int>::max());
	_ui->labelSimSpeedValue->setText(QString::number(_ui->horizontalSliderSimSpeed->value()));
	_ui->labelSimRefreshRateValue->setText(QString("%1 s").arg(_ui->horizontalSliderSimRefreshRate->value()));

	_ui->spinBoxQueueLenFixWorkers1->setMinimum(_ui->spinBoxWorkers1->value());
	_ui->spinBoxQueueLenFixWorkers1->setMaximum(_ui->spinBoxWorkers1->value());
	_ui->spinBoxQueueLenFixWorkers2->setMinimum(_ui->spinBoxWorkers2->value());
	_ui->spinBoxQueueLenFixWorkers2->setMaximum(_ui->spinBoxWorkers2->value());
	_ui->spinBoxTimeInServiceFixWorkers1->setMinimum(_ui->spinBoxWorkers1->value());
	_ui->spinBoxTimeInServiceFixWorkers1->setMaximum(_ui->spinBoxWorkers1->value());
	_ui->spinBoxTimeInServiceFixWorkers2->setMinimum(_ui->spinBoxWorkers2->value());
	_ui->spinBoxTimeInServiceFixWorkers2->setMaximum(_ui->spinBoxWorkers2->value());
	_prepare_plot();

	connect(_sim, SIGNAL(replication_started(int)), this, SLOT(replication_started(int)));
	connect(_sim, SIGNAL(replication_finished(int,replication_info)), this, SLOT(replication_finished(int,replication_info)));
	connect(_sim, SIGNAL(simulation_started(int,int)), this, SLOT(simulation_started(int,int)));
	connect(_sim, SIGNAL(simulation_finished(int,int,double,double,double,double)), this, SLOT(simulation_finished(int,int,double,double,double,double)));
	connect(_sim, SIGNAL(best_worker_count_found(int,int)), this, SLOT(best_worker_count_found(int,int)));
	connect(_sim, SIGNAL(run_finished()), this, SLOT(run_finished()));
	connect(_sim, SIGNAL(refresh_triggered(double,refresh_info)), this, SLOT(refresh_triggered(double,refresh_info)));
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
	if (checked)
	{
		_ui->spinBoxQueueLenFixWorkers1->setMinimum(_ui->spinBoxWorkers1Min->value());
		_ui->spinBoxQueueLenFixWorkers1->setMaximum(_ui->spinBoxWorkers1Max->value());
		_ui->spinBoxTimeInServiceFixWorkers1->setMinimum(_ui->spinBoxWorkers1Min->value());
		_ui->spinBoxTimeInServiceFixWorkers1->setMaximum(_ui->spinBoxWorkers1Max->value());
	}
	else
	{
		_ui->spinBoxQueueLenFixWorkers1->setMinimum(_ui->spinBoxWorkers1->value());
		_ui->spinBoxQueueLenFixWorkers1->setMaximum(_ui->spinBoxWorkers1->value());
		_ui->spinBoxTimeInServiceFixWorkers1->setMinimum(_ui->spinBoxWorkers1->value());
		_ui->spinBoxTimeInServiceFixWorkers1->setMaximum(_ui->spinBoxWorkers1->value());
	}
}

void MainWindow::on_checkBoxRangeWorkers2_toggled(bool checked)
{
	_ui->spinBoxWorkers2->setEnabled(!checked);
	_ui->spinBoxWorkers2Min->setEnabled(checked);
	_ui->spinBoxWorkers2Max->setEnabled(checked);
	if (checked)
	{
		_ui->spinBoxQueueLenFixWorkers2->setMinimum(_ui->spinBoxWorkers2Min->value());
		_ui->spinBoxQueueLenFixWorkers2->setMaximum(_ui->spinBoxWorkers2Max->value());
		_ui->spinBoxTimeInServiceFixWorkers2->setMinimum(_ui->spinBoxWorkers2Min->value());
		_ui->spinBoxTimeInServiceFixWorkers2->setMaximum(_ui->spinBoxWorkers2Max->value());
	}
	else
	{
		_ui->spinBoxQueueLenFixWorkers2->setMinimum(_ui->spinBoxWorkers2->value());
		_ui->spinBoxQueueLenFixWorkers2->setMaximum(_ui->spinBoxWorkers2->value());
		_ui->spinBoxTimeInServiceFixWorkers2->setMinimum(_ui->spinBoxWorkers2->value());
		_ui->spinBoxTimeInServiceFixWorkers2->setMaximum(_ui->spinBoxWorkers2->value());
	}
}

void MainWindow::on_pushButtonStartSimulation_clicked()
{
	_ui->pushButtonStartSimulation->setEnabled(false);
	_ui->pushButtonPauseResumeSimulation->setEnabled(true);
	_ui->pushButtonStopSimulation->setEnabled(true);
	_ui->groupBoxSeed->setEnabled(false);
	_ui->groupBoxParameters->setEnabled(false);
	_ui->spinBoxQueueLenFixWorkers1->setEnabled(false);
	_ui->spinBoxQueueLenFixWorkers2->setEnabled(false);
	_ui->spinBoxTimeInServiceFixWorkers1->setEnabled(false);
	_ui->spinBoxTimeInServiceFixWorkers2->setEnabled(false);

	_ui->widgetPlotQueueLenWorkers1->graph(0)->clearData();
	_ui->widgetPlotQueueLenWorkers1->replot();
	_ui->widgetPlotQueueLenWorkers2->graph(0)->clearData();
	_ui->widgetPlotQueueLenWorkers2->replot();
	_ui->widgetPlotTimeInServiceWorkers1->graph(0)->clearData();
	_ui->widgetPlotTimeInServiceWorkers1->replot();
	_ui->widgetPlotTimeInServiceWorkers2->graph(0)->clearData();
	_ui->widgetPlotTimeInServiceWorkers2->replot();

	auto replications = _ui->spinBoxReplications->value();
	auto unitIndex = _ui->comboBoxTimeUnit->currentIndex();
	auto time = funcs[unitIndex](_ui->spinBoxReplicationDuration->value());

	bool rangeWorkers1 = _ui->checkBoxRangeWorkers1->isChecked();
	bool rangeWorkers2 = _ui->checkBoxRangeWorkers2->isChecked();

	if (!rangeWorkers1 && !rangeWorkers2)
	{
		auto w1 = _ui->spinBoxWorkers1->value();
		auto w2 = _ui->spinBoxWorkers2->value();
		if (_ui->radioButtonCustomSeed->isChecked())
		{
			auto seed = static_cast<Seed>(_ui->spinBoxCustomSeed->value());
			_thr = std::thread(&sim_core_car_service::single_run_seed, _sim, replications, time, w1, w2, seed);
		}
		else
		{
			_thr = std::thread(&sim_core_car_service::single_run, _sim, replications, time, w1, w2);
		}
		return;
	}

	auto w1_min = _ui->spinBoxWorkers1->value(), w1_max = _ui->spinBoxWorkers1->value();
	if (rangeWorkers1)
	{
		w1_min = _ui->spinBoxWorkers1Min->value();
		w1_max = _ui->spinBoxWorkers1Max->value();
	}
	auto w2_min = _ui->spinBoxWorkers2->value(), w2_max = _ui->spinBoxWorkers2->value();
	if (rangeWorkers2)
	{
		w2_min = _ui->spinBoxWorkers2Min->value();
		w2_max = _ui->spinBoxWorkers2Max->value();
	}

	if (_ui->radioButtonCustomSeed->isChecked())
	{
		auto seed = static_cast<Seed>(_ui->spinBoxCustomSeed->value());
		_thr = std::thread(&sim_core_car_service::multi_run_seed, _sim, replications, time, w1_min, w1_max, w2_min, w2_max, seed);
	}
	else
	{
		_thr = std::thread(&sim_core_car_service::multi_run, _sim, replications, time, w1_min, w1_max, w2_min, w2_max);
	}
}

void MainWindow::on_pushButtonPauseResumeSimulation_clicked()
{
	auto state = _sim->get_state();
	_sim->pause_resume();

	if (state == sim_core_car_service::state::RUNNING)
	{
		_ui->pushButtonPauseResumeSimulation->setText(RESUME_TEXT);
	}
	else
	{
		_ui->pushButtonPauseResumeSimulation->setText(PAUSE_TEXT);
	}
}

void MainWindow::on_pushButtonStopSimulation_clicked()
{
	_sim->stop();
	_thr.join();

	_ui->pushButtonStartSimulation->setEnabled(true);
	_ui->pushButtonPauseResumeSimulation->setEnabled(false);
	_ui->pushButtonPauseResumeSimulation->setText(PAUSE_TEXT);
	_ui->pushButtonStopSimulation->setEnabled(false);
	_ui->groupBoxSeed->setEnabled(true);
	_ui->groupBoxParameters->setEnabled(true);
	_ui->spinBoxQueueLenFixWorkers1->setEnabled(true);
	_ui->spinBoxQueueLenFixWorkers2->setEnabled(true);
	_ui->spinBoxTimeInServiceFixWorkers1->setEnabled(true);
	_ui->spinBoxTimeInServiceFixWorkers2->setEnabled(true);
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

void MainWindow::on_spinBoxWorkers1_valueChanged(int value)
{
	_ui->spinBoxQueueLenFixWorkers1->setMinimum(value);
	_ui->spinBoxQueueLenFixWorkers1->setMaximum(value);
	_ui->spinBoxTimeInServiceFixWorkers1->setMinimum(value);
	_ui->spinBoxTimeInServiceFixWorkers1->setMaximum(value);
}

void MainWindow::on_spinBoxWorkers2_valueChanged(int value)
{
	_ui->spinBoxQueueLenFixWorkers2->setMinimum(value);
	_ui->spinBoxQueueLenFixWorkers2->setMaximum(value);
	_ui->spinBoxTimeInServiceFixWorkers2->setMinimum(value);
	_ui->spinBoxTimeInServiceFixWorkers2->setMaximum(value);
}

void MainWindow::on_spinBoxWorkers1Min_valueChanged(int value)
{
	_ui->spinBoxQueueLenFixWorkers1->setMinimum(value);
	_ui->spinBoxTimeInServiceFixWorkers1->setMinimum(value);
}

void MainWindow::on_spinBoxWorkers1Max_valueChanged(int value)
{
	_ui->spinBoxQueueLenFixWorkers1->setMaximum(value);
	_ui->spinBoxTimeInServiceFixWorkers1->setMaximum(value);
}

void MainWindow::on_spinBoxWorkers2Min_valueChanged(int value)
{
	_ui->spinBoxQueueLenFixWorkers2->setMinimum(value);
	_ui->spinBoxTimeInServiceFixWorkers2->setMinimum(value);
}

void MainWindow::on_spinBoxWorkers2Max_valueChanged(int value)
{
	_ui->spinBoxQueueLenFixWorkers2->setMaximum(value);
	_ui->spinBoxTimeInServiceFixWorkers2->setMaximum(value);
}

void MainWindow::replication_started(int replication)
{
	_ui->labelReplicationValue->setText(QString::number(replication));
}

void MainWindow::replication_finished(int replication, replication_info info)
{
	_ui->labelAVGCustomerQueueLenSimValue->setText(QString::number(info.average_customer_queue_length, 'f', 6));
	_ui->labelAVGQueueForRepairLenSimValue->setText(QString::number(info.average_cars_waiting_for_repair_queue_length, 'f', 6));
	_ui->labelAVGRepairedQueueLenSimValue->setText(QString::number(info.average_repaired_cars_queue_length, 'f', 6));
	_ui->labelWaitInQueueDurSimValue->setText(duration_as_string(info.average_wait_in_queue_duration));
	_ui->labelWaitForRepairDurSimValue->setText(duration_as_string(info.average_wait_for_repair_duration));
	_ui->labelTimeInServiceDurSimValue->setText(duration_as_string(info.average_time_in_service_duration));
	_ui->labelWaitForRepairCIValue->setText(QString("<%1, %2>").arg(duration_as_string(info.wait_for_repair_90_CI.first), duration_as_string(info.wait_for_repair_90_CI.second)));
	_ui->labelTimeInSystemCIValue->setText(QString("<%1, %2>").arg(duration_as_string(info.time_in_service_90_CI.first), duration_as_string(info.time_in_service_90_CI.second)));
	_ui->labelWaitInQueueCIValue->setText(QString("<%1, %2>").arg(duration_as_string(info.wait_in_queue_90_CI.first), duration_as_string(info.wait_in_queue_90_CI.second)));
}

void MainWindow::simulation_started(int workers1, int workers2)
{
	_ui->labelWorkers1CurrentValue->setText(QString::number(workers1));
	_ui->labelWorkers2CurrentValue->setText(QString::number(workers2));
	_ui->labelUsedSeedValue->setText(QString::number(_sim->get_seed()));
}

void MainWindow::simulation_finished(int workers1, int workers2, double wait_for_repair_total_time, double wait_in_queue_total_time, double queue_len_total, double time_in_service_total)
{
	if (workers1 == _ui->spinBoxQueueLenFixWorkers1->value())
	{
		_ui->widgetPlotQueueLenWorkers2->graph(0)->addData(workers2, queue_len_total);
		_ui->widgetPlotQueueLenWorkers2->rescaleAxes();
		_ui->widgetPlotQueueLenWorkers2->replot();
	}
	if (workers2 == _ui->spinBoxQueueLenFixWorkers2->value())
	{
		_ui->widgetPlotQueueLenWorkers1->graph(0)->addData(workers1, queue_len_total);
		_ui->widgetPlotQueueLenWorkers1->rescaleAxes();
		_ui->widgetPlotQueueLenWorkers1->replot();
	}
	if (workers1 == _ui->spinBoxTimeInServiceFixWorkers1->value())
	{
		_ui->widgetPlotTimeInServiceWorkers2->graph(0)->addData(workers2, to_hours(time_in_service_total));
		_ui->widgetPlotTimeInServiceWorkers2->rescaleAxes();
		_ui->widgetPlotTimeInServiceWorkers2->replot();
	}
	if (workers2 == _ui->spinBoxTimeInServiceFixWorkers2->value())
	{
		_ui->widgetPlotTimeInServiceWorkers1->graph(0)->addData(workers1, to_hours(time_in_service_total));
		_ui->widgetPlotTimeInServiceWorkers1->rescaleAxes();
		_ui->widgetPlotTimeInServiceWorkers1->replot();
	}
}

void MainWindow::best_worker_count_found(int w1, int w2)
{
	_ui->labelIdealWorkers1CountValue->setText(w1 == 0 ? "-" : QString::number(w1));
	_ui->labelIdealWorkers2CountValue->setText(w2 == 0 ? "-" : QString::number(w2));
}

void MainWindow::run_finished()
{
	_thr.join();

	_ui->pushButtonStartSimulation->setEnabled(true);
	_ui->pushButtonPauseResumeSimulation->setEnabled(false);
	_ui->pushButtonPauseResumeSimulation->setText(PAUSE_TEXT);
	_ui->pushButtonStopSimulation->setEnabled(false);
	_ui->groupBoxSeed->setEnabled(true);
	_ui->groupBoxParameters->setEnabled(true);
	_ui->spinBoxQueueLenFixWorkers1->setEnabled(true);
	_ui->spinBoxQueueLenFixWorkers2->setEnabled(true);
	_ui->spinBoxTimeInServiceFixWorkers1->setEnabled(true);
	_ui->spinBoxTimeInServiceFixWorkers2->setEnabled(true);
}

void MainWindow::refresh_triggered(double time, refresh_info info)
{
	_ui->labelTimeValue->setText(sim_time_as_string(time));
	_ui->labelWorkers1WorkingValue->setText(QString::number(info.workers_1_working));
	_ui->labelWorkers2WorkingValue->setText(QString::number(info.workers_2_working));
	_ui->labelCustomerQueueLenValue->setText(QString::number(info.customer_queue_length));
	_ui->labelQueueForRepairLenValue->setText(QString::number(info.cars_waiting_for_repair_queue_length));
	_ui->labelRepairedQueueLenValue->setText(QString::number(info.repaired_cars_queue_length));
	_ui->labelAVGCustomerQueueLenValue->setText(QString::number(info.average_customer_queue_length, 'f', 6));
	_ui->labelAVGQueueForRepairLenValue->setText(QString::number(info.average_cars_waiting_for_repair_queue_length, 'f', 6));
	_ui->labelAVGRepairedQueueLenValue->setText(QString::number(info.average_repaired_cars_queue_length, 'f', 6));
	_ui->labelWaitInQueueDurValue->setText(duration_as_string(info.average_wait_in_queue_duration));
	_ui->labelWaitForRepairDurValue->setText(duration_as_string(info.average_wait_for_repair_duration));
	_ui->labelTimeInServiceDurValue->setText(duration_as_string(info.average_time_in_service_duration));
}

void MainWindow::_prepare_plot()
{
	_ui->widgetPlotQueueLenWorkers1->clearGraphs();
	_ui->widgetPlotQueueLenWorkers1->addGraph()->setName("Priemerný počet čakajúcich v rade podľa počtu pracovníkov skupiny 1");
	_ui->widgetPlotQueueLenWorkers1->xAxis->setLabel("Počet pracovníkov skupiny 1");
	_ui->widgetPlotQueueLenWorkers1->yAxis->setLabel("Priemerný počet čakajúcich v rade");
	_ui->widgetPlotQueueLenWorkers1->plotLayout()->insertRow(0);
	auto plotQueueLen1Title = new QCPPlotTitle(_ui->widgetPlotQueueLenWorkers1, _ui->widgetPlotQueueLenWorkers1->graph(0)->name());
	_ui->widgetPlotQueueLenWorkers1->plotLayout()->addElement(0, 0, plotQueueLen1Title);
	_ui->widgetPlotQueueLenWorkers1->xAxis->setAutoTickStep(false);
	_ui->widgetPlotQueueLenWorkers1->xAxis->setTickStep(1.0);

	_ui->widgetPlotQueueLenWorkers2->clearGraphs();
	_ui->widgetPlotQueueLenWorkers2->addGraph()->setName("Priemerný počet čakajúcich v rade podľa počtu pracovníkov skupiny 2");
	_ui->widgetPlotQueueLenWorkers2->xAxis->setLabel("Počet pracovníkov skupiny 2");
	_ui->widgetPlotQueueLenWorkers2->yAxis->setLabel("Priemerný počet čakajúcich v rade");
	_ui->widgetPlotQueueLenWorkers2->plotLayout()->insertRow(0);
	auto plotQueueLen2Title = new QCPPlotTitle(_ui->widgetPlotQueueLenWorkers2, _ui->widgetPlotQueueLenWorkers2->graph(0)->name());
	_ui->widgetPlotQueueLenWorkers2->plotLayout()->addElement(0, 0, plotQueueLen2Title);
	_ui->widgetPlotQueueLenWorkers2->xAxis->setAutoTickStep(false);
	_ui->widgetPlotQueueLenWorkers2->xAxis->setTickStep(1.0);

	_ui->widgetPlotTimeInServiceWorkers1->clearGraphs();
	_ui->widgetPlotTimeInServiceWorkers1->addGraph()->setName("Priemerný čas strávený zákazníkom v servise podľa počtu pracovníkov skupiny 1");
	_ui->widgetPlotTimeInServiceWorkers1->xAxis->setLabel("Počet pracovníkov skupiny 1");
	_ui->widgetPlotTimeInServiceWorkers1->yAxis->setLabel("Priemerný čas strávený zákazníkom v servise (hodiny)");
	_ui->widgetPlotTimeInServiceWorkers1->plotLayout()->insertRow(0);
	auto plotTimeInService1Title = new QCPPlotTitle(_ui->widgetPlotTimeInServiceWorkers1, _ui->widgetPlotTimeInServiceWorkers1->graph(0)->name());
	_ui->widgetPlotTimeInServiceWorkers1->plotLayout()->addElement(0, 0, plotTimeInService1Title);
	_ui->widgetPlotTimeInServiceWorkers1->xAxis->setAutoTickStep(false);
	_ui->widgetPlotTimeInServiceWorkers1->xAxis->setTickStep(1.0);

	_ui->widgetPlotTimeInServiceWorkers2->clearGraphs();
	_ui->widgetPlotTimeInServiceWorkers2->addGraph()->setName("Priemerný čas strávený zákazníkom v servise podľa počtu pracovníkov skupiny 2");
	_ui->widgetPlotTimeInServiceWorkers2->xAxis->setLabel("Počet pracovníkov skupiny 2");
	_ui->widgetPlotTimeInServiceWorkers2->yAxis->setLabel("Priemerný čas strávený zákazníkom v servise (hodiny)");
	_ui->widgetPlotTimeInServiceWorkers2->plotLayout()->insertRow(0);
	auto plotTimeInService2Title = new QCPPlotTitle(_ui->widgetPlotTimeInServiceWorkers2, _ui->widgetPlotTimeInServiceWorkers2->graph(0)->name());
	_ui->widgetPlotTimeInServiceWorkers2->plotLayout()->addElement(0, 0, plotTimeInService2Title);
	_ui->widgetPlotTimeInServiceWorkers2->xAxis->setAutoTickStep(false);
	_ui->widgetPlotTimeInServiceWorkers2->xAxis->setTickStep(1.0);
}
