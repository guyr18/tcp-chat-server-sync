#pragma once
#include "Parameter.cpp"
#include <string>
#include <vector>
using std::string;
using std::vector;

class Command
{

    private:

        std::string m_commandName; // A string that represents the name of this command.
        std::vector<Parameter> m_parameters; // A vector of Parameter objects that represent the possibly parameters for this command.
        unsigned short m_numRequiredParams; // The number of Parameter objects marked as required inside m_parameters.
        std::string m_commandUsage; // A string literal that describes the abstract usage of this Command object.

    public:

        // Two-parameter constructor that accepts a command name and list of Parameter objects for that command.
        Command(const std::string& name, const std::vector<Parameter>& params, const std::string& usage, const unsigned short& nrp) noexcept : m_commandName{name}, m_parameters{params}, m_commandUsage{usage}, m_numRequiredParams{nrp} {}

        // One-parameter constructor that accepts a command name and initializes m_parameters to an empty list. This is useful
        // if you would like to manually add parameters using the addParameter(..) method.
        explicit Command(const std::string& name, const std::string& usage) noexcept : m_commandName{name}, m_commandUsage{usage}, m_numRequiredParams{0} {}

        // Default destructor.
        ~Command() {}

        // AddParameter(param) appends Parameter object param to m_parameters.
        void addParameter(const Parameter& param)
        {

            m_parameters.push_back(param);

            if(param.isRequired())
            {

                m_numRequiredParams++;

            }
        }

        // GetCommand() returns the name of this Command object; m_commandName.
        inline const std::string getCommandName() const noexcept
        {

            return m_commandName;

        }

        // GetParameterList() returns the list Parameter objects binded to this Command instance; m_parameters.
        inline const std::vector<Parameter>& getParameterList() noexcept
        {

            return m_parameters;

        }

        // GetNumRequiredParams() returns the number of required parameters for this Command object; m_numRequiredParams.
        inline const unsigned short getNumRequiredParams() noexcept
        {

            return m_numRequiredParams;

        }

        // GetCommandUsage() returns a string that describes the usage of this Command object; m_commandUsage.
        inline const std::string getCommandUsage() const noexcept
        {

            return m_commandUsage;

        }
};