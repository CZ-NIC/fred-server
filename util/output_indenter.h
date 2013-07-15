#ifndef OUTPUT_INDENTER_H__
#define OUTPUT_INDENTER_H__

#include <ostream>


struct OutputIndenter
{
    unsigned short indent;
    unsigned short level;
    char c;

    OutputIndenter dive() const
    {
        return OutputIndenter(indent, level + 1, c);
    }

    OutputIndenter(unsigned short _i, unsigned short _l, char _c)
        : indent(_i), level(_l), c(_c)
    {
    }

    friend std::ostream& operator<<(std::ostream &_ostream, const OutputIndenter &_oi);
};



#endif /*OUTPUT_INDENTER_H__*/

