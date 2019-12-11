#ifndef _PLOCALALIGNER_H_
#define _PLOCALALIGNER_H_
#include "localaligner.h"

template <class Similarity_Matrix_Type, class LocalAligner_Type>
class OMPParallelLocalAligner : public ParallelLocalAligner<Similarity_Matrix_Type, LocalAligner_Type> {
  public:
    OMPParallelLocalAligner(std::string_view, std::string_view, int, float);
    OMPParallelLocalAligner(std::string_view, std::string_view, int, float, float);
    OMPParallelLocalAligner(std::string_view, std::string_view, int, float, std::function<float(const char &, const char &)> &&);
    OMPParallelLocalAligner(std::string_view, std::string_view, int, float, std::function<float(const char &, const char &)> &&, float);
    float calculateScore() override;
    float getScore() const override { return max_score; };
    unsigned int getPos() const override { return pos; };
    std::string_view getConsensus_x() const override { return consensus_x; };
    std::string_view getConsensus_y() const override { return consensus_y; };
    Eigen::VectorXf getTimings() const override { return sm_timings; };
  private:
    Eigen::VectorXf sm_timings;
    unsigned int pos;
    float max_score;
    float gap_penalty;
    float overlap_ratio; // heuristics of maximum len(consensus_x)/len(sequence_x)
    int npiece; // number of pieces that sequence_y is broken into
    std::string consensus_x;
    std::string consensus_y;
    std::string_view sequence_x;
    std::string_view sequence_y;
    std::function<float(const char &, const char &)> scoring_function;
};

#endif //_PLOCALALIGNER_H_
