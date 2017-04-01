QT += \
	core \
	gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SEM02
TEMPLATE = app
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
	DEFINES += DEBUG
	DEFINES += LOG_FILE_NAME=\\\"$$TARGET\\\"
	DESTDIR = ../debug/bin/
	OBJECTS_DIR = ../debug/.obj/
	LIBS += -L../debug/lib/
}

CONFIG(release, debug|release) {
	DESTDIR = ../release/bin/
	OBJECTS_DIR = ../release/.obj/
	LIBS += -L../release/lib/
}

INCLUDEPATH += ../include/

win32:CONFIG(debug, debug|release): LIBS += -lqcustomplotd1
else:win32:CONFIG(release, debug|release): LIBS += -lqcustomplot1
else:unix: LIBS += -lqcustomplot

SOURCES += \
	main.cpp \
	MainWindow.cpp \
	log/log.cpp \
	pool/object_pool.cpp \
	stat/statistic.cpp
	gen/empirical_int_distribution_generator.cpp \
	gen/triangular_distribution_generator.cpp \
	sim/customer.cpp \
	sim/sim_core_base.cpp \
	sim/sim_event_base.cpp \
	sim/sim_core_car_service.cpp \
	sim/sim_events_car_service.cpp \
	sim/sim_wrapper.cpp \

HEADERS += \
	MainWindow.hpp \
	log/log.hpp \
	pool/object_pool.hpp \
	stat/statistic.hpp
	gen/empirical_int_distribution_generator.hpp \
	gen/triangular_distribution_generator.hpp \
	sim/customer.hpp \
	sim/sim_settings.hpp \
	sim/sim_core_base.hpp \
	sim/sim_event_base.hpp \
	sim/sim_core_car_service.hpp \
	sim/sim_events_car_service.hpp \
	sim/sim_wrapper.hpp \

FORMS += \
	MainWindow.ui
