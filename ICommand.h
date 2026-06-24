#pragma once

#include <string>

class ICommand
{
public:
    enum CommandType {
        PRINT
    };

    ICommand(int pid, CommandType commandType);
    virtual ~ICommand() = default;

    CommandType getCommandType() const;
    virtual void execute(int coreId, const std::string& processName, const std::string& outputFile) = 0;

protected:
    int pid;
    CommandType commandType;
};

inline ICommand::CommandType ICommand::getCommandType() const
{
    return this->commandType;
}

inline ICommand::ICommand(int pid, CommandType commandType)
{
    this->pid = pid;
    this->commandType = commandType;
}