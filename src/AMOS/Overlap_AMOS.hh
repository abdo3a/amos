////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 06/18/2003
//!
//! \brief Header for Overlap_t
//!
////////////////////////////////////////////////////////////////////////////////

#ifndef __Overlap_AMOS_HH
#define __Overlap_AMOS_HH 1

#include "Universal_AMOS.hh"
#include <utility>




namespace AMOS {

typedef char OverlapAdjacency_t;

//================================================ Overlap_t ===================
//! \brief An overlap relation between to sequencing reads
//!
//! A similarity based overlap between two Read_t that suggests their relative
//! location (to one another) in the original sequence.
//!
//==============================================================================
class Overlap_t : public Universal_t
{

public:

  static const OverlapAdjacency_t NULL_ADJACENCY = 0;
  static const OverlapAdjacency_t NORMAL     = 'N';     //!< E,B
  static const OverlapAdjacency_t ANTINORMAL = 'A';     //!< B,E
  static const OverlapAdjacency_t INNIE      = 'I';     //!< E,E
  static const OverlapAdjacency_t OUTIE      = 'O';     //!< B,B


private:

  Size_t aHang_m;          //!< length of non-overlapping portion of A
  Size_t bHang_m;          //!< length of non-overlapping portion of B
  std::pair<ID_t, ID_t> reads_m;   //!< the pair of read IDs


protected:

  static const uint8_t FIRST_BIT  = 0x1;  //!< adjacency info for 1st read
  static const uint8_t SECOND_BIT = 0x2;  //!< adjacency info for 2nd read


  //--------------------------------------------------- readRecord -------------
  virtual Size_t readRecord (std::istream & fix,
			     std::istream & var);


  //--------------------------------------------------- writeRecord ------------
  virtual Size_t writeRecord (std::ostream & fix,
			      std::ostream & var) const;


public:

  //--------------------------------------------------- NCode ------------------
  //! \brief Get the AMOS NCode type identifier (statically)
  //!
  //! Can be used for constructing a Bank with a certain NCode. e.g. 'Bank_t
  //! (Overlap_t::NCode( ))'
  //!
  //! \return The AMOS NCode type identifier
  //!
  static NCode_t NCode ( )
  {
    return Bank_k::OVERLAP;
  }


  //--------------------------------------------------- Overlap_t --------------
  //! \brief Constructs an empty Overlap_t object
  //!
  //! Sets all members to 0
  //!
  Overlap_t ( )
  {
    aHang_m = bHang_m = 0;
    reads_m . first = reads_m . second = NULL_ID;
  }


  //--------------------------------------------------- Overlap_t --------------
  //! \brief Copy constructor
  //!
  Overlap_t (const Overlap_t & source)
  {
    *this = source;
  }


  //--------------------------------------------------- ~Overlap_t -------------
  //! \brief Destroys a Overlap_t object
  //!
  ~Overlap_t ( )
  {

  }


  //--------------------------------------------------- clear ------------------
  virtual void clear ( )
  {
    Universal_t::clear( );
    aHang_m = bHang_m = 0;
    reads_m . first = reads_m . second = NULL_ID;
  }


  //--------------------------------------------------- getAdjacency -----------
  //! \brief Get the overlapping ends of the reads
  //!
  //! Get the overlap information for the reads, i.e. [N]ORMAL (EB),
  //! [A]NTINORMAL (BE), [I]NNIE (EE) or [O]UTIE (BB). Where B is the
  //! beginning of the read and E is the end of the read and [N]ORMAL means
  //! the end of read1 overlaps the beginning of read2.
  //!
  //! \return The pair of adjacent ends
  //!
  OverlapAdjacency_t getAdjacency ( ) const;


  //--------------------------------------------------- getAhang ---------------
  //! \brief Get the A-hang overlap value
  //!
  //! Get the length of the non-overlapping portion of the first read.
  //!
  //! \return The A-hang overlap value
  //!
  Size_t getAhang ( ) const
  {
    return aHang_m;
  }


  //--------------------------------------------------- getBhang ---------------
  //! \brief Get the B-hang overlap value
  //!
  //! Get the length of the non-overlapping portion of the second read.
  //!
  //! \return The B-hang overlap value
  //!
  Size_t getBhang ( ) const
  {
    return bHang_m;
  }


  //--------------------------------------------------- getNCode ---------------
  virtual NCode_t getNCode ( ) const
  {
    return Overlap_t::NCode( );
  }


  //--------------------------------------------------- getReads ---------------
  //! \brief Get the pair of contig IDs joined by this overlap
  //!
  //! \return The first and second read IDs joined by this overlap
  //!
  std::pair<ID_t, ID_t> getReads ( ) const
  {
    return reads_m;
  }


  //--------------------------------------------------- setAdjacency -----------
  //! \brief Set the overlapping ends of the reads
  //!
  //! Set the overlap information for the reads, i.e. [N]ORMAL (EB),
  //! [A]NTINORMAL (BE), [I]NNIE (EE) or [O]UTIE (BB). Where B is the
  //! beginning of the read and E is the end of the read and EB would mean
  //! the end of read1 overlaps the beginning of read2.
  //!
  //! \note Will store info in extra portion of BankableFlags
  //!
  //! \param adj The new adjacency of the reads
  //! \pre adj must be one of [NAIO]
  //! \throws ArgumentException_t
  //! \return void
  //!
  void setAdjacency (OverlapAdjacency_t adj);


  //--------------------------------------------------- setAhang ---------------
  //! \brief Set the A-hang overlap value
  //!
  //! Set the length of the non-overlapping portion of the first read.
  //!
  //! \param aHang The new A-hang overlap value
  //! \return The new A-hang overlap value
  //!
  void setAhang (Size_t aHang)
  {
    aHang_m = aHang;
  }


  //--------------------------------------------------- setBhang ---------------
  //! \brief Set the B-hang overlap value
  //!
  //! Set the length of the non-overlapping portion of the second read.
  //!
  //! \param bHang The new B-hang overlap value
  //! \return The new B-hang overlap value
  //!
  void setBhang (Size_t bHang)
  {
    bHang_m = bHang;
  }


  //--------------------------------------------------- setReads ---------------
  //! \brief Set the read ID pair for this overlap
  //!
  //! \param reads The new pair of read IDs
  //!
  void setReads (std::pair<ID_t, ID_t> reads)
  {
    reads_m = reads;
  }

};

} // namespace AMOS

#endif // #ifndef __Overlap_AMOS_HH
