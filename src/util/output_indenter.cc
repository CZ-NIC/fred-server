#include "src/util/output_indenter.hh"


std::ostream& operator<<(std::ostream &_ostream, const OutputIndenter &_oi)
{
    return _ostream << std::string(_oi.indent * _oi.level, _oi.c);
}

