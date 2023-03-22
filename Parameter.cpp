#pragma once
#include <string>
using std::string;

class Parameter
{

    private:

        std::string m_parameterName;
        bool m_required;

    public:

        // Two-parameter constructor that accepts a parameter name and the default required status of this parameter.
        // By default, required is set to true.
        explicit Parameter(const std::string& name, const bool& required = true) noexcept : m_parameterName{name}, m_required{required} {}

        // Default destructor.
        ~Parameter() {}

        // GetParameterName() returns the name of this Parameter object; m_parameterName.
        inline const std::string getParameterName() const noexcept
        {

            return m_parameterName;

        }

        // IsRequired() returns true if this Parameter object is required. And false otherwise; m_required.
        inline bool isRequired() const noexcept
        {

            return m_required;

        }
        
};
