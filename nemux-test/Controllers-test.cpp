#include "gtest/gtest.h"

#include "Controllers.h"

using namespace std;

////////////////////////////////////////////////////////////////

struct ControllersTest : public ::testing::Test {
    Controllers controllers;
    void SetP1(bool up, bool down, bool left, bool right,
               bool start, bool select, bool a, bool b) {
        controllers.P1_Up = up;
        controllers.P1_Down = down;
        controllers.P1_Left = left;
        controllers.P1_Right = right;
        controllers.P1_Start = start;
        controllers.P1_Select = select;
        controllers.P1_A = a;
        controllers.P1_B = b;
    }
    
    bool ReadP1() {
        return IsBitSet<0>(controllers.ReadP1());
    }

    void Expect_P1(bool up, bool down, bool left, bool right,
        bool start, bool select, bool a, bool b) {
        EXPECT_EQ(a,      ReadP1()) << "Read A";
        EXPECT_EQ(b,      ReadP1()) << "Read B";
        EXPECT_EQ(select, ReadP1()) << "Read Select";
        EXPECT_EQ(start,  ReadP1()) << "Read Start";
        EXPECT_EQ(up,     ReadP1()) << "Read Up";
        EXPECT_EQ(down,   ReadP1()) << "Read Down";
        EXPECT_EQ(left,   ReadP1()) << "Read Left";
        EXPECT_EQ(right,  ReadP1()) << "Read Right";
    }
};

TEST_F(ControllersTest, ReadP1_InitialState) {
    Expect_P1(false, false, false, false, false, false, false, false);
}

TEST_F(ControllersTest, ReadP1_StandardRead) {
    SetP1(true, false, true, false, true, false, true, false);
    controllers.Write(0x01);
    controllers.Write(0x00);
    Expect_P1(true, false, true, false, true, false, true, false);
}

TEST_F(ControllersTest, ReadP1_StandardReadOtherTiming) {
    controllers.Write(0x01);
    SetP1(true, false, true, false, true, false, true, false);
    controllers.Write(0x00);
    Expect_P1(true, false, true, false, true, false, true, false);
}

TEST_F(ControllersTest, ReadP1_OverflowState) {
    SetP1(true, false, true, false, true, false, true, false);
    controllers.Write(0x01);
    controllers.Write(0x00);
    Expect_P1(true, false, true, false, true, false, true, false);
    Expect_P1(true, true, true, true, true, true, true, true);
}

TEST_F(ControllersTest, ReadP1_StrobeActive) {
    controllers.Write(0x01);
    EXPECT_EQ(false, ReadP1());
    controllers.P1_A = true;
    EXPECT_EQ(true, ReadP1());
    EXPECT_EQ(true, ReadP1());
    controllers.P1_A = false;
    EXPECT_EQ(false, ReadP1());
    EXPECT_EQ(false, ReadP1());
}

TEST_F(ControllersTest, ReadP1_DataIsLatched) {
    SetP1(true, false, true, false, true, false, true, false);
    controllers.Write(0x01);
    controllers.Write(0x00);
    SetP1(false, true, false, true, false, true, false, true);

    Expect_P1(true, false, true, false, true, false, true, false);
}

TEST_F(ControllersTest, ReadP1_DataIsLatchedExtraStopStrobe) {
    SetP1(true, false, true, false, true, false, true, false);
    controllers.Write(0x01);
    controllers.Write(0x00);
    SetP1(false, true, false, true, false, true, false, true);
    controllers.Write(0x00);

    Expect_P1(true, false, true, false, true, false, true, false);
}

TEST_F(ControllersTest, ReadP1_D1D2DataLines) {
    controllers.Write(0x01);
    controllers.P1_A = false;

    EXPECT_EQ(0x00, controllers.ReadP1() & 0x06);

    controllers.P1_A = true;

    EXPECT_EQ(0x00, controllers.ReadP1() & 0x06);
}

TEST_F(ControllersTest, ReadP1_D3D4DataLines) {
    controllers.Write(0x01);
    controllers.P1_A = false;

    EXPECT_EQ(0x00, controllers.ReadP1() & 0x18);

    controllers.P1_A = true;

    EXPECT_EQ(0x00, controllers.ReadP1() & 0x18);
}

TEST_F(ControllersTest, ReadP1_D5D6D7OpenBus) {
    FAIL();
}
