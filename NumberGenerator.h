#pragma once
#include <random>
#include <iostream>
#include <limits>


//https://en.cppreference.com/w/cpp/types/numeric_limits/max
//https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
static class NumberGenerator
{
public:
    static int GenerateRandomInteger(int lower, int upper);
    static float GenerateRandomFloat(float lower, float upper);
    static double GenerateRandomDouble(double lower, double upper);
};

