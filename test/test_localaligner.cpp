#include<gtest/gtest.h>

#include "localaligner.h"
#include "smithwaterman.h"

TEST(LocalAligner,SmallSeqAlignment) {
  // https://en.wikipedia.org/wiki/Smith%E2%80%93Waterman_algorithm#/media/File:Smith-Waterman-Algorithm-Example-Step2.png
  std::string sequence_x = "GGTTGACTA";
  std::string sequence_y = "TGTTACGG";

  std::string_view expected_consensus_x = "CAGTTG";
  std::string_view expected_consensus_y = "CA-TTG";

  LocalAligner *la = new SWAligner(sequence_x, sequence_y);

  auto maxval = la->calculateScore();
  auto pos = la->getPos();

  ASSERT_EQ(la->getConsensus_x(), expected_consensus_x);
  ASSERT_EQ(la->getConsensus_y(), expected_consensus_y);

  ASSERT_EQ(maxval, 13);
  ASSERT_EQ(pos, 2);
}
