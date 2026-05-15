#ifndef PRIMARYGENERATORINFO_H
#define PRIMARYGENERATORINFO_H

#include <string>

class PrimaryGeneratorInfo {
public:
    virtual ~PrimaryGeneratorInfo() {}
    virtual std::string GetGeneratorName() const = 0;
    virtual std::string GetInfoSummary() const = 0;
    virtual void Initialize() {}
};

#endif