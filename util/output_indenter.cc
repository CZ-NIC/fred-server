#include "output_indenter.h"


std::ostream& operator<<(std::ostream &_ostream, const OutputIndenter &_oi)
{
    return _ostream << std::string(_oi.indent * _oi.level, _oi.c);
}

