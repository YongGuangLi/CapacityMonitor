/**
 * Autogenerated by Thrift Compiler (0.10.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef CapacityMonitor_TYPES_H
#define CapacityMonitor_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace com { namespace thriftcode {

class data_info;

class CalcException;


class data_info : public virtual ::apache::thrift::TBase {
 public:

  data_info(const data_info&);
  data_info& operator=(const data_info&);
  data_info() : colNum(0), colName(), minValue(0), maxValue(0) {
  }

  virtual ~data_info() throw();
  int32_t colNum;
  std::string colName;
  int32_t minValue;
  int32_t maxValue;

  void __set_colNum(const int32_t val);

  void __set_colName(const std::string& val);

  void __set_minValue(const int32_t val);

  void __set_maxValue(const int32_t val);

  bool operator == (const data_info & rhs) const
  {
    if (!(colNum == rhs.colNum))
      return false;
    if (!(colName == rhs.colName))
      return false;
    if (!(minValue == rhs.minValue))
      return false;
    if (!(maxValue == rhs.maxValue))
      return false;
    return true;
  }
  bool operator != (const data_info &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const data_info & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(data_info &a, data_info &b);

inline std::ostream& operator<<(std::ostream& out, const data_info& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _CalcException__isset {
  _CalcException__isset() : code(false), message(false), dateTime(false) {}
  bool code :1;
  bool message :1;
  bool dateTime :1;
} _CalcException__isset;

class CalcException : public ::apache::thrift::TException {
 public:

  CalcException(const CalcException&);
  CalcException& operator=(const CalcException&);
  CalcException() : code(0), message(), dateTime() {
  }

  virtual ~CalcException() throw();
  int32_t code;
  std::string message;
  std::string dateTime;

  _CalcException__isset __isset;

  void __set_code(const int32_t val);

  void __set_message(const std::string& val);

  void __set_dateTime(const std::string& val);

  bool operator == (const CalcException & rhs) const
  {
    if (__isset.code != rhs.__isset.code)
      return false;
    else if (__isset.code && !(code == rhs.code))
      return false;
    if (__isset.message != rhs.__isset.message)
      return false;
    else if (__isset.message && !(message == rhs.message))
      return false;
    if (__isset.dateTime != rhs.__isset.dateTime)
      return false;
    else if (__isset.dateTime && !(dateTime == rhs.dateTime))
      return false;
    return true;
  }
  bool operator != (const CalcException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const CalcException & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
  mutable std::string thriftTExceptionMessageHolder_;
  const char* what() const throw();
};

void swap(CalcException &a, CalcException &b);

inline std::ostream& operator<<(std::ostream& out, const CalcException& obj)
{
  obj.printTo(out);
  return out;
}

}} // namespace

#endif
