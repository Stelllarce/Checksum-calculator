#pragma once

#include "directory-iteration-visitors/VerificationVisitor.hpp"
#include <map>
#include <string>
#include <iostream>

class VerificationResultPrinter
{
public:
    void printResults(const std::map<std::string, VerificationStatus> &results, std::ostream &os = std::cout);
};
