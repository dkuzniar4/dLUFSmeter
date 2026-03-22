#pragma once
#include <cmath>
#include <array>

#ifndef M_PI
namespace
{
    const double M_PI = std::acos(-1.0);
}
#endif

class LUFSMeterJUCE
{
public:
    static constexpr int MAX_CHANNELS = 2;
    static constexpr int MAX_SR = 192000;
    static constexpr int MAX_WINDOW_SAMPLES = MAX_SR * 3; // 3 sek

    void prepare(double sampleRate, int numChannels)
    {
        sr = sampleRate;
        ch = numChannels;

        momentarySamples = int(0.4 * sr);
        shortTermSamples = int(3.0 * sr);

        reset();

        for (int c = 0; c < ch; ++c)
        {
            hp[c].setHighPass(sr);
            hs[c].setHighShelf(sr);
        }
    }

    void reset()
    {
        totalEnergy = 0.0;
        totalSamples = 0;

        momentaryIndex = 0;
        shortIndex = 0;

        momentaryCount = 0;
        shortCount = 0;

        for (auto& f : hp) f.reset();
        for (auto& f : hs) f.reset();
    }

    // JUCE: call inside processBlock
    void process(const float* const* buffer, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            double energy = 0.0;

            for (int c = 0; c < ch; ++c)
            {
                double x = buffer[c][i];

                x = hp[c].process(x);
                x = hs[c].process(x);

                energy += x * x;
            }

            energy /= ch;

            if (!freezeIntegrated)
            {
                totalEnergy += energy;
                totalSamples++;
            }

            pushMomentary(energy);
            pushShortTerm(energy);
        }
    }

    // ======================
    // LUFS READOUTS
    // ======================

    float getMomentaryLUFS() const
    {
        if (momentaryCount == 0) return -100.0f;
        return toLUFS(momentarySum / momentaryCount);
    }

    float getShortTermLUFS() const
    {
        if (shortCount == 0) return -100.0f;
        return toLUFS(shortSum / shortCount);
    }

    float getIntegratedLUFS() const
    {
        if (totalSamples == 0) return -100.0f;
        return toLUFS(totalEnergy / totalSamples);
    }

    void setIntegratedFreeze(bool shouldFreeze)
    {
        freezeIntegrated = shouldFreeze;
    }

private:
    // ======================
    // FILTER
    // ======================

    struct Biquad
    {
        double a0{}, a1{}, a2{}, b1{}, b2{};
        double z1{}, z2{};

        void reset() { z1 = z2 = 0.0; }

        void setHighPass(double sr)
        {
            double f = 60.0;
            double Q = 0.5;

            double K = tan(M_PI * f / sr);
            double norm = 1.0 / (1.0 + K / Q + K * K);

            a0 = norm;
            a1 = -2.0 * norm;
            a2 = norm;
            b1 = 2.0 * (K * K - 1.0) * norm;
            b2 = (1.0 - K / Q + K * K) * norm;
        }

        void setHighShelf(double sr)
        {
            double f = 6500.0;
            double A = pow(10.0, 4.0 / 40.0);

            double w0 = 2.0 * M_PI * f / sr;
            double alpha = sin(w0) / 2.0 * sqrt(2.0);

            double cosw = cos(w0);

            double b0 = A * ((A + 1) + (A - 1) * cosw + 2 * sqrt(A) * alpha);
            double b1_ = -2 * A * ((A - 1) + (A + 1) * cosw);
            double b2_ = A * ((A + 1) + (A - 1) * cosw - 2 * sqrt(A) * alpha);
            double a0_ = (A + 1) - (A - 1) * cosw + 2 * sqrt(A) * alpha;
            double a1_ = 2 * ((A - 1) - (A + 1) * cosw);
            double a2_ = (A + 1) - (A - 1) * cosw - 2 * sqrt(A) * alpha;

            a0 = b0 / a0_;
            a1 = b1_ / a0_;
            a2 = b2_ / a0_;
            b1 = a1_ / a0_;
            b2 = a2_ / a0_;
        }

        inline double process(double x)
        {
            double y = a0 * x + z1;
            z1 = a1 * x - b1 * y + z2;
            z2 = a2 * x - b2 * y;
            return y;
        }
    };

    // ======================
    // RING BUFFERS
    // ======================

    void pushMomentary(double v)
    {
        momentarySum -= momentaryBuffer[momentaryIndex];
        momentaryBuffer[momentaryIndex] = v;
        momentarySum += v;

        momentaryIndex = (momentaryIndex + 1) % momentarySamples;

        if (momentaryCount < momentarySamples)
            momentaryCount++;
    }

    void pushShortTerm(double v)
    {
        shortSum -= shortBuffer[shortIndex];
        shortBuffer[shortIndex] = v;
        shortSum += v;

        shortIndex = (shortIndex + 1) % shortTermSamples;

        if (shortCount < shortTermSamples)
            shortCount++;
    }

    inline float toLUFS(double meanSquare) const
    {
        return float(-0.691 + 10.0 * log10(meanSquare + 1e-12) + 3.7);
    }

    // ======================
    // STATE
    // ======================

    double sr{};
    int ch{};

    double totalEnergy{};
    long long totalSamples{};

    std::array<Biquad, MAX_CHANNELS> hp;
    std::array<Biquad, MAX_CHANNELS> hs;

    // Momentary
    std::array<double, MAX_WINDOW_SAMPLES> momentaryBuffer{};
    int momentarySamples{};
    int momentaryIndex{};
    int momentaryCount{};
    double momentarySum{};

    // Short-term
    std::array<double, MAX_WINDOW_SAMPLES> shortBuffer{};
    int shortTermSamples{};
    int shortIndex{};
    int shortCount{};
    double shortSum{};

    // Integrated
    bool freezeIntegrated = false;
};