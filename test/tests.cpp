// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include "TimedDoor.h"

// Mock classes

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

// Fixture

class TimedDoorTest : public ::testing::Test {
 protected:
  TimedDoor* door;

  void SetUp() override {
    door = new TimedDoor(1);
  }

  void TearDown() override {
    delete door;
  }
};

// Tests

TEST_F(TimedDoorTest, DoorInitiallyClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorOpensOnUnlock) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorClosesOnLock) {
  door->unlock();
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
  EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, ThrowStateThrowsWhenDoorOpen) {
  door->unlock();
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateNoThrowWhenDoorClosed) {
  EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenDoorOpen) {
  door->unlock();
  DoorTimerAdapter adapter(*door);
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutNoThrowWhenDoorClosed) {
  DoorTimerAdapter adapter(*door);
  EXPECT_NO_THROW(adapter.Timeout());
}

TEST_F(TimedDoorTest, TimerCallsTimeoutOnClient) {
  MockTimerClient mockClient;
  EXPECT_CALL(mockClient, Timeout()).Times(1);
  Timer timer;
  timer.tregister(1, &mockClient);
}

TEST_F(TimedDoorTest, TimerThrowsForOpenDoorAfterTimeout) {
  door->unlock();
  DoorTimerAdapter adapter(*door);
  Timer timer;
  EXPECT_THROW(timer.tregister(1, &adapter), std::runtime_error);
}

TEST_F(TimedDoorTest, NoThrowIfDoorClosedBeforeTimeout) {
  door->unlock();
  door->lock();
  DoorTimerAdapter adapter(*door);
  Timer timer;
  EXPECT_NO_THROW(timer.tregister(1, &adapter));
}

TEST_F(TimedDoorTest, MockDoorInterfaceVerification) {
  MockDoor mockDoor;
  EXPECT_CALL(mockDoor, lock()).Times(1);
  EXPECT_CALL(mockDoor, unlock()).Times(1);
  EXPECT_CALL(mockDoor, isDoorOpened()).Times(1)
    .WillOnce(::testing::Return(false));

  mockDoor.lock();
  mockDoor.unlock();
  EXPECT_FALSE(mockDoor.isDoorOpened());
}
