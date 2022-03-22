#include "NumberGenerator.h"

int NumberGenerator::GenerateRandomInteger(int lower, int upper)
{
    if (lower == 0 && upper == 0)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(0, std::numeric_limits<float>::max());
        return dist6(rng);
    }
    else
    {
        if (lower < 0 && upper > 0)
        {
            int diffForZero = std::abs(lower - 0);
            lower += diffForZero;
            upper += diffForZero;
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist6(lower, upper);
            return dist6(rng) - diffForZero;
        }
        else
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist6(lower, upper);
            return dist6(rng);
        }
    }
}

float NumberGenerator::GenerateRandomFloat(float lower, float upper)
{
    if (lower == 0 && upper == 0)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
        return dist6(rng);
    }
    else
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(lower, upper);
        return dist6(rng);
    }
}

double NumberGenerator::GenerateRandomDouble(double lower, double upper)
{
    if (lower == 0 && upper == 0)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
        return dist6(rng);
    }
    else
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(lower, upper);
        return dist6(rng);
    }
}
