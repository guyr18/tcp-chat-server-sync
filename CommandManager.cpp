#pragma once
#include "Command.cpp"
#include <vector>
using std::vector;

class CommandManager
{

    private:

        CommandManager() {} // Default parameterless constructor; hidden to enforce singleton use (@see getInstance()).

        std::vector<Command> m_commands; // A vector of command objects that represent a user command.

    public:

        // Default destructor.
        ~CommandManager() {}

        // GetInstance() is a singleton method that returns a static instance of this class. Only one
        // instance should be created during the lifetime of the application.
        static CommandManager& getInstance()
        {

            static CommandManager inst;
            return inst;

        }

        // GetCommandList() returns a list of cached Command objects; m_commands.
        inline const std::vector<Command>& getCommandList() noexcept
        {

            return m_commands;

        }

        // AddCommand(cmd) appends Command object cmd to m_commands.
        void addCommand(const Command& cmd)
        {

            m_commands.push_back(cmd);

        }

        // IsValidCommandName(name) returns true if name is a valid command name within m_commands. And false otherwise.
        bool isValidCommandName(const std::string& name)
        {

            // Iterate through m_commands.
            for(auto itr = m_commands.begin(); itr != m_commands.end(); itr++)
            {

                // If this Command object's name matches parameter name, return true.
                if((*itr).getCommandName() == name)
                {

                    return true;

                }
            }

            // No match found. Return false.
            return false;

        }

        // GetParameterListOf(name) returns a pair containing the Command object and list of Parameter objects for the command that has
        // a name of name. If no such command name exists, an empty string Command an empty vector will be returned.
        std::pair<Command, std::vector<Parameter>> getParameterListOf(const std::string& name)
        {

            // Iterate through m_commands.
            for(auto itr = m_commands.begin(); itr != m_commands.end(); itr++)
            {

                // If this Command object's name matches parameter name, return corresponding lvalue pair.
                if((*itr).getCommandName() == name)
                {

                    return std::pair<Command, std::vector<Parameter>>(*itr, (*itr).getParameterList());

                }
            }

            // No match found. Return empty pair.
            return std::pair<Command, std::vector<Parameter>>(Command("", ""), std::vector<Parameter>());

        }

        bool inputMatchesCommandParamList(const std::string& input, const std::string& name)
        {

            size_t curParamIndex = 0;
            size_t paramsMatched = 0;
            auto commandAndParam = getParameterListOf(name);

            // Iterate through input string.
            for(size_t index = 0; index < input.length(); index++)
            {

                // Spaces are acceptable for the final parameter. So as a result, we don't
                // need to evaluate the remaining input. Just increment paramsMatched and break.
                if(curParamIndex == commandAndParam.second.size() - 1)
                {

                    paramsMatched++;
                    break;

                }

                // If this is a whitespace character that is not found at index 0, we have
                // iterated through a parameter at this point. Increment paramsMatched and curParamIndex to reflect that.
                if(input[index] == ' ' && index > 0)
                {
                    
                    paramsMatched++;
                    curParamIndex++;

                }
            }

            // Did we at least match the required number of parameters? Return this boolean expression to find out.
            return paramsMatched >= commandAndParam.first.getNumRequiredParams();

        }
};