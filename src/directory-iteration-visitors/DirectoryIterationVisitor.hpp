#pragma once
#include <iostream>

class Directory;
class File;
class Link;

class DirectoryIterationVisitor {
public:
    virtual ~DirectoryIterationVisitor() = default;

    virtual void visitFile(File& file) { }

    virtual void visitDirectory(Directory& dir) { }
    
    virtual void visitLink(Link& link) { }

    void processFile(File& file);
protected:
    DirectoryIterationVisitor(std::ostream& os);

    virtual void preProcess(File& file) { }
    virtual void applyAlgorithm(File& file) { }
    virtual void postProcess(File& file) { }
protected:
    std::ostream& _output; // For testing
};