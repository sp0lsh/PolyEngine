#include "EnginePCH.hpp"

#include "Time/TimeSystem.hpp"
#include <chrono>

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

using namespace Poly;

//------------------------------------------------------------------------------
void TimeSystem::TimeUpdatePhase(World * world)
{
	TimeWorldComponent* timeComponent = world->GetWorldComponent<TimeWorldComponent>();

	//Get new delta time
	std::chrono::steady_clock::time_point frameTime = steady_clock::now();
	duration<double> deltaTime = duration_cast<duration<double>>(frameTime - timeComponent->LastFrameTime);

	//In future this should be surrounded by #ifndef FINAL
	deltaTime = deltaTime > duration_cast<duration<double>>(timeComponent->MIN_TIME_STEP_MS) ? timeComponent->MIN_TIME_STEP_MS : deltaTime;
	//#endif

	timeComponent->LastFrameTime = frameTime;

	//Update timers
	for (auto& timer : timeComponent->Timers)
	{
		if (timeComponent->Paused && timer.second.IsPausable)
		{
			timer.second.DeltaTime = 0.0;
			continue;
		}
		deltaTime *= timer.second.Multiplier;
		timer.second.Time += deltaTime;
		timer.second.DeltaTime = deltaTime.count();
	}
}

//------------------------------------------------------------------------------
void TimeSystem::RegisterTimer(World * world, size_t id, bool isPausable, double multiplier)
{
	TimeWorldComponent* timeComponent = world->GetWorldComponent<TimeWorldComponent>();
	ASSERTE(timeComponent->Timers.count(id) == 0, "Same timer id provided twice!");
	timeComponent->Timers[id] = Timer(isPausable, multiplier);
}

//------------------------------------------------------------------------------
double TimeSystem::GetTimerDeltaTime(World * world, size_t id)
{
	TimeWorldComponent* timeComponent = world->GetWorldComponent<TimeWorldComponent>();
	if(timeComponent->Timers.count(id) == 0)
		throw std::invalid_argument("Timer with given id does not exist.");
	else 
		return timeComponent->Timers[id].GetDeltaTime();
}

//------------------------------------------------------------------------------
double TimeSystem::GetTimerDeltaTime(World * world, eEngineTimer timerType)
{
	return world->GetWorldComponent<TimeWorldComponent>()->Timers.at((size_t)timerType).GetDeltaTime();
}

//------------------------------------------------------------------------------
double TimeSystem::GetTimerElapsedTime(World * world, size_t id)
{
	TimeWorldComponent* timeComponent = world->GetWorldComponent<TimeWorldComponent>();
	if(timeComponent->Timers.count(id) == 0)
		throw std::invalid_argument("Timer with given id does not exist.");
	else 
		return timeComponent->Timers[id].GetTime();
}

//------------------------------------------------------------------------------
double TimeSystem::GetTimerElapsedTime(World * world, eEngineTimer timerType)
{
	return world->GetWorldComponent<TimeWorldComponent>()->Timers.at((size_t)timerType).GetTime();
}

//------------------------------------------------------------------------------
double TimeSystem::GetTimerMultiplier(World * world, size_t id)
{
	TimeWorldComponent* timeComponent = world->GetWorldComponent<TimeWorldComponent>();
	if(timeComponent->Timers.count(id) == 0)
		throw std::invalid_argument("Timer with given id does not exist.");
	else 
		return timeComponent->Timers[id].GetMultiplier();
}

//------------------------------------------------------------------------------
double TimeSystem::GetTimerMultiplier(World * world, eEngineTimer timerType)
{
	return world->GetWorldComponent<TimeWorldComponent>()->Timers.at((size_t)timerType).GetMultiplier();
}

void TimeSystem::SetTimerMultiplier(World* world, size_t id, double multiplier)
{
	TimeWorldComponent* timeComponent = world->GetWorldComponent<TimeWorldComponent>();
	if (timeComponent->Timers.count(id) == 0)
		throw std::invalid_argument("Timer with given id does not exist.");
	else
		timeComponent->Timers[id].SetMultiplier(multiplier);
}