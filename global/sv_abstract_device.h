﻿#ifndef SV_ABSTRACT_EDVICE_H
#define SV_ABSTRACT_EDVICE_H

#include <QObject>
#include <QRunnable>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QTimer>

#include "../virtual_device/virtual_device_global.h"
#include "../../svlib/sv_exception.h"
#include "../../svlib/sv_abstract_logger.h"

#include "sv_signal.h"
//#include "device_params.h"

#define MAX_PACKET_SIZE 0xFFFF

namespace ad {

  struct DeviceInfo
  {
    int id = -1;
    QString name = "";
    QString hardware_name = "";
//    HardwareType hardware_type = HardwareType::UNDEFINED_TYPE;
//    IfcType ifc_type = IfcType::UNDEFINED_IFC;
    QString ifc_name = "";
    QString ifc_params = "";
    QString device_params = "";
    QString driver_lib_name = "";
    QString description = "";
    bool is_involved = false;
    bool debug_mode = false;
    bool debug2_mode = false;
    quint64 timeout = 0;
  };


  struct BUFF
  {
    BUFF() {}

    quint8  buf[MAX_PACKET_SIZE];
    quint64 offset = 0;

  };

  struct DATA
  {
    DATA() {}

    qint8  data[MAX_PACKET_SIZE];
    quint8  data_type;
    quint8  data_length;
    quint16 crc;

  };

  typedef QMap<QString, SvSignal*> SignalMap;

  class SvAbstractDevice;
  class SvAbstractDeviceThread;

}
    
class ad::SvAbstractDevice: public QObject
{
  Q_OBJECT
  
public:
  SvAbstractDevice(sv::SvAbstractLogger* logger = nullptr):
    p_thread(nullptr),
    p_logger(logger)
  {
    clearSignals();
  }

/* обязательно виртуальй деструктор, чтобы вызывались деструкторы наследников */
  virtual ~SvAbstractDevice()
  {
//    deleteLater();
  }
  
//  virtual dev::HardwareType type() const { return p_hardware_type; }
  
  virtual void create_new_thread() throw(SvException) = 0;
  virtual ad::SvAbstractDeviceThread* thread() const { return p_thread; }

  virtual void setLogger(sv::SvAbstractLogger* logger) { p_logger = logger; }
  virtual const sv::SvAbstractLogger* logger() const { return p_logger; }

  virtual bool configure(const ad::DeviceInfo& info) = 0;

  virtual const ad::DeviceInfo* info() const { return &p_info; }
//  virtual const ad::DeviceParams* params() const { return &p_params; }

  virtual bool open() = 0;

  virtual void close() = 0;
  virtual void stop()   { }

  void setLastError(const QString& lastError) { p_last_error = lastError; }
  const QString &lastError() const            { return p_last_error; }

  void setOpened(bool isOpened) { p_isOpened = isOpened; }
  bool isOpened() const         { return p_isOpened; }

  void setActive(bool isActive) { p_isActive = isActive; }
  bool isActive() const         { return p_isActive; }

  bool isAlive() { return p_lost_epoch > quint64(QDateTime::currentMSecsSinceEpoch());  }

  /** работа с сигналами, привязанными к устройству **/
//  void addSignal(SvSignal* signal) { p_signals.insert(signal->params()->name, signal); }
  void addSignal(SvSignal* signal)  { p_signals.insert(signal->params()->name, signal); }
  void clearSignals()               { p_signals.clear(); }
  int  signalsCount() const         { return p_signals.count(); }

  const ad::SignalMap* Signals() const { return &p_signals; }

  inline void setSignalValue(const QString& signal_name, qreal value)
  {
    if(p_signals.contains(signal_name)) {

//      qDebug() << QString("SIGNAL_NAME: %1   VALUE: %2").arg(SIGNAL_NAME).arg(VALUE);
      p_signals.value(signal_name)->setValue(value);

    }
  }

  inline void setNewLostEpoch()
  {
      p_lost_epoch = QDateTime::currentMSecsSinceEpoch() + p_info.timeout;

      foreach (SvSignal* s, p_signals.values())
        s->setDeviceLostEpoch(p_lost_epoch);
  }

protected:
//  dev::HardwareType p_hardware_type;

  ad::SvAbstractDeviceThread* p_thread = nullptr;

  ad::DeviceInfo    p_info;

  sv::SvAbstractLogger* p_logger;

  ad::SignalMap p_signals;

  SvException p_exception;
  QString p_last_error;

  bool p_isOpened = false;
  bool p_isActive = true;
  bool p_is_ready_read = false;

  quint64 p_lost_epoch = 0;
  
};


class ad::SvAbstractDeviceThread: public QThread
{
  Q_OBJECT
  
public:
  SvAbstractDeviceThread(ad::SvAbstractDevice* device, sv::SvAbstractLogger* logger = nullptr):
    p_logger(logger),
    p_device(device)
  {  }

  /* обязательно виртуальй деструктор, чтобы вызывались деструкторы наследников */
  ~SvAbstractDeviceThread() { }

  virtual void conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException) = 0;
//  virtual void setIfcParams(const QString& params) throw(SvException&) = 0;

  virtual void open() throw(SvException) = 0;
  virtual quint64 write(const QByteArray& data) = 0;

  virtual void setLogger(sv::SvAbstractLogger* logger)
  {
    p_logger = logger;
  }
  
protected:
  sv::SvAbstractLogger  *p_logger = nullptr;
  ad::SvAbstractDevice  *p_device = nullptr;

  bool p_is_active;

  ad::BUFF p_buff;
  ad::DATA p_data;

  QTimer  p_reset_timer;

  SvException p_exception;

  virtual void process_data() = 0;

public slots:

public slots:
  virtual void reset_buffer()
  {
    p_buff.offset = 0;
  }

  virtual void stop() = 0;
//  virtual void stop()
//  {
//    p_is_active = false;
//  }

};


#endif // SV_ABSTRACT_EDVICE_H
