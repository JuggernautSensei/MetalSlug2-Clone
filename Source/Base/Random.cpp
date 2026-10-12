#include "Base/Random.h"

RngMt19937::RngMt19937(const uint64_t _seed)
    : m_mt(_seed)
{
}

void RngMt19937::Reseed(const uint64_t _seed)
{
    m_mt.seed(_seed);
}

uint64_t RngMt19937::Generate()
{
    return m_mt();
}
