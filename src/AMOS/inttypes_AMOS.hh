////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 11/12/2003
//!
//! \brief Include file for all the AMOS integer types
//!
////////////////////////////////////////////////////////////////////////////////

#ifndef __inttypes_AMOS_HH
#define __inttypes_AMOS_HH 1

#include <inttypes.h>       //!< ints of the form [u]int(size)_t, i.e. int32_t
#include <string>

typedef uint32_t  ID_t;     //!< ID type for iid's
typedef uint32_t  NCode_t;  //!< unique integer code for dynamic typing
typedef uint32_t  SD_t;     //!< standard deviation type
typedef int32_t   Size_t;   //!< size type (for links,etc)
typedef int32_t   Pos_t;    //!< position type (in a sequence,etc)
                            //-- Pos_t must be >= Size_t

namespace AMOS {

//----------------------------------------------------- Decode -----------------
//! \brief Converts an NCode integer to an AMOS message/field type
//!
//! \param ncode The NCode to translate to a string
//! \return String representation of the NCode
//!
std::string Decode (NCode_t ncode);


//----------------------------------------------------- Encode -----------------
//! \brief Converts an AMOS message/field type to an NCode integer
//!
//! \param str The NCode string equivalent
//! \pre str is of proper NCode length, i.e. str . size( ) == NCODE_SIZE
//! \return NCode of the string
//!
NCode_t Encode (const std::string & str);


const uint8_t NCODE_SIZE  =  3;         //!< length of the NCode strings
const NCode_t NULL_NCODE  =  0;         //!< NULL NCode definition
const ID_t    NULL_ID     =  0;         //!< NULL ID definition
const char    MIN_QUALITY = '0';        //!< min quality score definition
const char    MAX_QUALITY = '0' + 63;   //!< max quality score definition
const char    NULL_CHAR   = '\0';       //!< null char
const char    NL_CHAR     = '\n';       //!< newline char
const string  NULL_STRING = &NULL_CHAR; //!< null string

const ID_t    MAX_ID      = (2 << 31) - 1;   //!< 2^32 - 1 (unsigned 32bit int)
const Size_t  MAX_SIZE    = (2 << 30) - 1;   //!< 2^31 - 1 (signed 32bit int)
const Pos_t   MAX_POS     = (2 << 30) - 1;   //!< 2^31 - 1 (signed 32bit int)

} // namespace AMOS

#endif // #ifndef __inttypes_AMOS_HH
