#include "foundation_AMOS.hh"
#include <fstream>
#include <ctime>
using namespace std;
using namespace AMOS;

int main (int argc, char ** argv)
{
  Message_t msg;
  Read_t red;
  Contig_t ctg;

  double loopa = 0;
  double loopb = 0;
  clock_t clocka, clockb;

  vector<Tile_t> tlevec;
  Tile_t tle;

  tle . source = 33396;
  tle . offset = 512;
  tle . range = Range_t (12,832);
  tle . gaps . push_back (12);
  tle . gaps . push_back (30);
  tle . gaps . push_back (337);
  tle . gaps . push_back (508);
  tle . gaps . push_back (514);
  tle . gaps . push_back (789);

  for ( int j = 0; j < 200; j ++ )
    tlevec . push_back (tle);

  try {

    red . setIID (1);
    red . setEID ("str2");
    red . setComment ("GBRAA01TF");
    red . setSequence
      ("CCTTTGTGCTGGAAGGTGAATGCGGTGCGGCTGGTAACGGGCCGCGCGGGCGCAATCGGCGTCGTGATGGGCCGCAACAGCGAGTTTCACTTTGCCGAATTCATGCGCGGCATGGCCGAACGGCTGGGGCAGGACGAAGTGGACATTCTCGTCAGCCCCACCTCGCCGACCGGCAATGACGACGAGGTGCAGCTTTGTCACCGGCTGGCAACGAGCGGACGGGTGGATGCCGTTATCGTCACTTCCCCCAGGCCGAATGATGAACGCATCCGCATGTTGCACAAGCTCGGCATGCCGTTTCTGGTCCACGGGCGTTCGCAGATCGACATTCCCCATGCATGGCTTGATATCGACAATGAGAGCGCGGTCTATCATTCCACCGCACATCTGCTCGATCTTGGCCACAAGCGGATTGCGATGATCAACGGGCGCCATGGCTTCACCTTCTCGCTGCACCGCGACGCTGGCTATCGTAAGGCGCTTGAAAGCCGGGGAATCGACTTTGATCCCGACCTGGTGGAACATGGCGATTTCACCGATGAAATCGGCTATTGCCTCGCCCGCAAGCTTCTGGAACGCAATCCACGTCCAACGGCATTCGTCGCAGGCTCCATGATGACCGCGCTCGGCATCTACCGTGCCGCCCGCTCCTTCGGCCTTACGATCGGCAAGGATATTTCCGTCATCGCCCATGACGACGTGATCTCGTTCCTCAGCGCCGGTAATATGGGTGCCGTCGCTGACTGCGACACGCTCTTCAATGCGCGCGGCTGGCAAGCGATGCGCCGACCTTTTGATGGATATCCTCGATGGGCGCGCGCCCACCGAAATCCA",
       "9878<?JQONBB77:=EGMPDOHRNSTSTSTRSSSREMRSSRSSRSQSSLQLLLLMPPPRRSSSRRRRSSSRSSSRRSRRRRSRIJJJONLTTTTRRROLLLLLLTTTTTTTRRRRPPPSRRSTTTRSRRRSPOLOOLOSSSSRSSRSRPRPPPSPSTSTSQSQTRURSSSSSTTTTRRRSRRSRSSSURRRRSWSSRRRSSTSRRTTTTTTTTTPPSRPPSTTTTRSRUSSSSUSPPPPRSQRTTTTSSSRRRRRTRRSTRTTTRTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTPSPPPPPRRRTTTTTRSSSSRRSRRSTSTTTTTTTTRTTRSSSTRSRSRRSSSTTTRSSRSSSUSSQSSRSSSSRRRRRSRRTRTSRTSURTTTTTRSSSSSSTRTRRRSSRSSRTSPSPPPRTRRSSRRSTRRRSSRRRRTTTRRRUSRSSSTRSSSSRRLLLLOLNOTTTSRRRRSSSTTTTTTTTTTTWRRPPPPPPSTTTTWPPMSTPTTTTTTTTSTSRSRSSTTRSSSSTTTTTTTTPPPPKPPSRSSSTSSSSTSSRRSSSTTTTTRRSSTTSSRSSSSSTTTTTTTTTTTRTTTSRSSSSSTTTTRRRSSPPSPPPTTTTSRRPORSRPPPPLPPSSSSTTSSRSSSRPPNSSQRTTTTTSSPKKGMNPSSQSSRSSSSQLLIL?HSTQPSSPLRSQSSRSUQSMRSSUPRLLPGEPPSNMRPTYRRQPMMGGPMLQE?CENRNCRSLQRNTTKTKLQTXSRSRTTPRQXISRCONK<?IDTPPPDDIR?B<LALBDL@;LQQLCH?E=@IC<9F9?I;9;=8");
    red . setFragment (1);
    red . setClearRange (Range_t(16,823));
    red . setVectorClearRange (Range_t(16,823));
    red . setQualityClearRange (Range_t(16,823));

    ctg . setIID (1);
    ctg . setEID ("str2");
    ctg . setComment ("big contig");

    ctg . setSequence
      ("CCTTTGTGCTGGAAGGTGAATGCGGTGCGGCTGGTAACGGGCCGCGCGGGCGCAATCGGCGTCGTGATGGGCCGCAACAGCGAGTTTCACTTTGCCGAATTCATGCGCGGCATGGCCGAACGGCTGGGGCAGGACGAAGTGGACATTCTCGTCAGCCCCACCTCGCCGACCGGCAATGACGACGAGGTGCAGCTTTGTCACCGGCTGGCAACGAGCGGACGGGTGGATGCCGTTATCGTCACTTCCCCCAGGCCGAATGATGAACGCATCCGCATGTTGCACAAGCTCGGCATGCCGTTTCTGGTCCACGGGCGTTCGCAGATCGACATTCCCCATGCATGGCTTGATATCGACAATGAGAGCGCGGTCTATCATTCCACCGCACATCTGCTCGATCTTGGCCACAAGCGGATTGCGATGATCAACGGGCGCCATGGCTTCACCTTCTCGCTGCACCGCGACGCTGGCTATCGTAAGGCGCTTGAAAGCCGGGGAATCGACTTTGATCCCGACCTGGTGGAACATGGCGATTTCACCGATGAAATCGGCTATTGCCTCGCCCGCAAGCTTCTGGAACGCAATCCACGTCCAACGGCATTCGTCGCAGGCTCCATGATGACCGCGCTCGGCATCTACCGTGCCGCCCGCTCCTTCGGCCTTACGATCGGCAAGGATATTTCCGTCATCGCCCATGACGACGTGATCTCGTTCCTCAGCGCCGGTAATATGGGTGCCGTCGCTGACTGCGACACGCTCTTCAATGCGCGCGGCTGGCAAGCGATGCGCCGACCTTTTGATGGATATCCTCGATGGGCGCGCGCCCACCGAAATCCA",
       "9878<?JQONBB77:=EGMPDOHRNSTSTSTRSSSREMRSSRSSRSQSSLQLLLLMPPPRRSSSRRRRSSSRSSSRRSRRRRSRIJJJONLTTTTRRROLLLLLLTTTTTTTRRRRPPPSRRSTTTRSRRRSPOLOOLOSSSSRSSRSRPRPPPSPSTSTSQSQTRURSSSSSTTTTRRRSRRSRSSSURRRRSWSSRRRSSTSRRTTTTTTTTTPPSRPPSTTTTRSRUSSSSUSPPPPRSQRTTTTSSSRRRRRTRRSTRTTTRTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTPSPPPPPRRRTTTTTRSSSSRRSRRSTSTTTTTTTTRTTRSSSTRSRSRRSSSTTTRSSRSSSUSSQSSRSSSSRRRRRSRRTRTSRTSURTTTTTRSSSSSSTRTRRRSSRSSRTSPSPPPRTRRSSRRSTRRRSSRRRRTTTRRRUSRSSSTRSSSSRRLLLLOLNOTTTSRRRRSSSTTTTTTTTTTTWRRPPPPPPSTTTTWPPMSTPTTTTTTTTSTSRSRSSTTRSSSSTTTTTTTTPPPPKPPSRSSSTSSSSTSSRRSSSTTTTTRRSSTTSSRSSSSSTTTTTTTTTTTRTTTSRSSSSSTTTTRRRSSPPSPPPTTTTSRRPORSRPPPPLPPSSSSTTSSRSSSRPPNSSQRTTTTTSSPKKGMNPSSQSSRSSSSQLLIL?HSTQPSSPLRSQSSRSUQSMRSSUPRLLPGEPPSNMRPTYRRQPMMGGPMLQE?CENRNCRSLQRNTTKTKLQTXSRSRTTPRQXISRCONK<?IDTPPPDDIR?B<LALBDL@;LQQLCH?E=@IC<9F9?I;9;=8");
    ctg . setReadTiling (tlevec);

    clocka = clock( );
    for ( int i = 0; i < 1000000; i ++ )
      ctg . writeMessage (msg);
    clockb = clock( );
    loopa = (double)(clockb - clocka);

    clocka = clock( );
    for ( int i = 0; i < 1000000; i ++ )
      ctg . readMessage (msg);
    clockb = clock( );
    loopb = (double)(clockb - clocka);

    msg . setField ("eid", "hello");
    msg . setField ("bla", "foo");
    msg . write (cerr);

    cerr << endl
	 << "loopa: " << (double)loopa / CLOCKS_PER_SEC << " sec.\n"
	 << "loopb: " << (double)loopb / CLOCKS_PER_SEC << " sec.\n"
	 << "granu: " << CLOCKS_PER_SEC << " of a sec.\n";
  }
  catch (Exception_t & e) {
    cerr << "ERROR: exception\n" << e;
  }

  return 0;
}
