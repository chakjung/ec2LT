#pragma once

#include <map>
#include <set>

#include <aws/ec2/model/Instance.h> // Instance

class Entry;

class AZ {
public:
  Aws::String AZId;
  Aws::EC2::Model::Instance Instance;

  std::map<const Aws::String, Entry> DST;
};

class Entry {
public:
  unsigned long long RESCount = 0;
  unsigned long long RESSum = 0;

  unsigned long long HDSCount = 0;
  unsigned long long HDSSum = 0;

  unsigned long long RTTSum = 0;
  std::multiset<unsigned long long> RTT;

  void AddRES(const std::string &);
  void AddRES(const char *);

  void AddHDS(const std::string &);
  void AddHDS(const char *);

  void AddRTT(const std::string &);
  void AddRTT(const char *);

private:
  void _AddRTT(const unsigned long long &);
};
