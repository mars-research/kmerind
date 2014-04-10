/**
 * @file		kmer_functions.h
 * @ingroup
 * @author	tpan
 * @brief
 * @details
 *
 * Copyright (c) 2014 Georgia Institute of Technology.  All Rights Reserved.
 *
 * TODO add License
 */
#ifndef KMER_FUNCTIONS_H_
#define KMER_FUNCTIONS_H_

#include <vector>
#include <tuple>

namespace bliss
{
  namespace index
  {
	  
	/**
     * template Param:  T is type of data
     *                  P is number of bins to hash to.
     */
    template<typename T>
    struct XorModulus
    {
		T p;
		T offset;
		XorModulus(T _p, T _offest) : p(_p), offset(_offset) {};
		
		T operator()(const T &v1, const T &v2) {
			return (v1 ^ v2) % p + offset;
		}
    };
	
    // TODO:  need to change the templates.
    // FASTQ_SEQUENCE should be parameterized with base iterator and alphabet - DONE
    // given those + k, should be able to get kmer index element type  - DONE
    // given kmer index element type, and fastq sequence type, should be able to specialize kmer generator type.  DONE
    // given the kmer index element type, should be able to define sendbuffer type.  But won't.  SendBuffer may be of different types.

    // this can become a 1 to n transformer???
    template<typename KmerGenOp, typename SendBuffer, typename HashFunc>
    class KmerIndexGenerator {
      public:
        typedef typename KmerGenOp::SequenceType		SequenceType;
        typedef std::vector<SendBuffer>  				    BufferType;
        typedef bliss::iterator::buffered_transform_iterator<KmerGenOp, typename KmerGenOp::BaseIterType> KmerIter;
        typedef typename KmerGenOp::KmerType 			  KmerType;
		typedef typename KmerGenOp::OutputType KmerIndexPairType;

        KmerIndexGenerator(int nprocs, int rank, int tid) :
          _nprocs(nprocs), _rank(rank), _tid(tid), h(nprocs, nprocs * tid)  {
          static_assert(std::is_same<SequenceType,
                        typename SendBuffer::ValueType>::value,
                        "Kmer Generation and Send Buffer should use the same type");
        }

      protected:
        int _nprocs;
        int _rank;
        int _tid;
		HashFunc h;
        constexpr int K = KmerType::KmerSize::K;

        void operator()(SeqType &read, int j, BufType &buffers, std::vector<size_t> &counts) {
          KmerGenOp kmer_op(read.id);
          KmerIter start(read.seq, kmer_op);
          KmerIter end(read.seq_end, kmer_op);

          KmerIndexPairType index_kmer;
          //uint64_t kmerCount;


          // NOTE: need to call *start to actually evaluate.  question is whether ++ should be doing computation.
          for (int i = 0; start != end; ++start, ++i)
          {

            if (i < (K - 1))
              continue;

            index_kmer = *start;

            // some debugging output
            // printf("kmer send to %lx, key %lx, pos %d, qual %f\n", index_kmer.first, index_kmer.second.kmer, index_kmer.second.id.components.pos, index_kmer.second.qual);

            // sending the kmer.
            //printf("rank %d thread %d, staging to buffer %d\n", rank, tid, index_kmer.first % nprocs + (nprocs * tid) );

            // TODO: abstract this to hide details.
            buffers[h(index_kmer.first.kmer, index_kmer.second)].buffer(index_kmer.first);
            //      printf("kmer send to %lx, key %lx, pos %d, qual %f\n", index_kmer.first, index_kmer.second.kmer, index_kmer.second.id.components.pos, index_kmer.second.qual);
            //++kmerCount;
            counts[tid] += 1;
          }
        }
    };

    template<typename SendBuffer, typename KmerGenOp, typename QualGenOp, typename HashFunc>
    class KmerIndexGeneratorQuality : public KmerIndexGenerator<SendBuffer,
                                          KmerGenOp, HashFunc>
    {
      public:
        typedef KmerIndexGeneratorNoQuality<SendBuffer, KmerGenOp, HashFunc> BaseType;
        typedef typename BaseType::SequenceType SequenceType;
        typedef typename BaseType::BufferType BufferType;
        typedef typename BaseType::KmerIter KmerIter;
        typedef typename BaseType::KmerType KmerType;
        typedef bliss::iterator::buffered_transform_iterator<QualGenOp,
            typename QualGenOp::BaseIterType> QualIter;
		typedef typename KmerGenOp::OutputType KmerIndexPairType;

        KmerIndexGeneratorQuality(int nprocs, int rank, int tid)
            : BaseType(nprocs, rank, tid)
        {
          static_assert(std::is_same<SequenceType, typename QualGenOp::SequenceType>::value, "Kmer Generation and Quality Generation should use the same type");
        }

      protected:

        void operator()(SeqType &read, int j, BufType &buffers, std::vector<size_t> &counts) {

          KmerGenOp kmer_op(read.id);
          KmerIter start(read.seq, kmer_op);
          KmerIter end(read.seq_end, kmer_op);

          QualGenOp qual_op;
          QualIter qstart(read.qual, qual_op);
          QualIter qend(read.qual_end, qual_op);

          KmerIndexPairType index_kmer;
          //uint64_t kmerCount;


          // NOTE: need to call *start to actually evaluate.  question is whether ++ should be doing computation.
          for (int i = 0; (start != end) && (qstart != qend); ++start, ++qstart, ++i)
          {

            if (i < (K - 1))
              continue;

            index_kmer = *start;
            index_kmer.first.qual = *qstart;

            // some debugging output
            // printf("kmer send to %lx, key %lx, pos %d, qual %f\n", index_kmer.first, index_kmer.second.kmer, index_kmer.second.id.components.pos, index_kmer.second.qual);

            // sending the kmer.
            //printf("rank %d thread %d, staging to buffer %d\n", rank, tid, index_kmer.first % nprocs + (nprocs * tid) );
            if (fabs(index_kmer.first.qual) > std::numeric_limits<
                    typename KmerType::QualityType>::epsilon())
            {
              // sending the kmer.
              //printf("rank %d thread %d, staging to buffer %d\n", rank, tid, index_kmer.first % nprocs + (nprocs * tid) );

              // TODO: abstract this to hide details.
              buffers[h(index_kmer.first.kmer, index_kmer.second)].buffer(index_kmer.first);
              //      printf("kmer send to %lx, key %lx, pos %d, qual %f\n", index_kmer.first, index_kmer.second.kmer, index_kmer.second.id.components.pos, index_kmer.second.qual);
              // ++kmerCount;
              counts[tid] += 1;
            }
            else
            {
              //      printf("BAD kmer quality.  key %lx, pos %d, qual %f\n", index_kmer.second.kmer, index_kmer.second.id.components.pos, index_kmer.second.qual);
            }

          }
        }

    };


  } // namespace index

} // namespace bliss



#endif /* KMER_FUNCTIONS_H_ */
