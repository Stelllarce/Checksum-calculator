#include "VerificationResultPrinter.hpp"

void VerificationResultPrinter::printResults(const std::map<std::string, VerificationStatus> &results, std::ostream &os)
{
    for (const auto &pair : results)
    {
        os << pair.first << ": ";
        switch (pair.second)
        {
        case VerificationStatus::OK:
            os << "OK";
            break;
        case VerificationStatus::MODIFIED:
            os << "MODIFIED";
            break;
        case VerificationStatus::NEW:
            os << "NEW";
            break;
        case VerificationStatus::REMOVED:
            os << "REMOVED";
            break;
        }
        os << std::endl;
    }
}
