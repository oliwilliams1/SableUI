#pragma once
#include <SableUI/component.h>
#include <SableUI/states/timer.h>
#include <SableUI/states/worker.hpp>
#include <SableUI/SableUI.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include <string>
#include <SableUI/components/button.h>
#include <SableUI/utils.h>
#include <SableUI/worker_pool.h>
#include <SableUI/events.h>
#include <SableUI/scrollContext.h>

using namespace SableUI::Style;

namespace SableUI {

	struct ComputeResult {
		int value;
		std::string operation;
		int durationMs;
	};

	// test component from gemini
	class WorkerTestComponent : public BaseComponent {
	public:
		WorkerTestComponent() {
			clockInterval.Start();
			blinkInterval.Start();

			heavyWorker.SetTask([this]() {
				return PerformHeavyComputation();
			});

			lightWorker.SetTask([this]() {
				return PerformLightComputation();
			});
		}

		void Layout() override {
			ScrollViewCtx(scrollCtx, w_fill, h_fill, bg(25, 25, 25))
			{
				Div(w_fill, h_fill, p(20)) {
					Text("Worker Pool & Timer System Test",
						fontSize(28), mb(20), textColour(255, 255, 255));

					// === TIMER SECTION ===
					Div(mb(30), p(15), bg(35, 35, 40), rounded(8), w_fill) {
						Text("Timer System", fontSize(20), mb(15), textColour(100, 200, 255));

						Div(mb(15), left_right) {
							Div(w_fit) {
								Text("Current Time:", mb(5), textColour(150, 150, 150));
								Text(currentTime.get(), fontSize(24), textColour(100, 200, 255));
							}
							Div(w(20), h(1));
							Div(w_fit) {
								Text("Uptime:", mb(5), textColour(150, 150, 150));
								Text(SableString::Format("%d seconds", uptime.get()),
									fontSize(24), textColour(255, 200, 100));
							}
						}

						Div(left_right, mb(15)) {
							Colour blinkColour = (blink.get()) ? Colour(0, 255, 100) : Colour(40, 80, 50);
							Rect(w(16), h(16),
								bg(blinkColour),
								rounded(8), mr(10), centerY);
							Text("Interval (500ms) - Blink Status",
								textColour(180, 180, 180), centerY, wrapText(false));
						}

						if (!timeoutMessage.get().empty()) {
							Div(p(10), bg(80, 50, 120), rounded(5), mb(10)) {
								Text(timeoutMessage.get(), textColour(255, 200, 255));
							}
						}

						Div(left_right) {
							ButtonWithVariant("Reset Uptime", [this]() {
								uptime.set(0);
								}, ButtonVariant::Secondary, w_fit);

							Div(w(10), h(1));

							ButtonWithVariant(clockInterval.IsRunning() ? "Pause Clock" : "Resume Clock",
								[this]() {
									if (clockInterval.IsRunning()) {
										clockInterval.Stop();
										blinkInterval.Stop();
									}
									else {
										clockInterval.Start();
										blinkInterval.Start();
									}
								},
								clockInterval.IsRunning() ? ButtonVariant::Danger : ButtonVariant::Primary,
								w_fit);

							Div(w(10), h(1));

							ButtonWithVariant("Trigger 3s Timeout", [this]() {
								timeoutMessage.set("Timeout scheduled...");
								testTimeout.Schedule(3.0f, [this]() {
									timeoutMessage.set("Timeout fired after 3 seconds!");
									});
								}, ButtonVariant::Primary, w_fit);
						}
					}

					// === WORKER SECTION ===
					Div(mb(30), p(15), bg(35, 35, 40), rounded(8), w_fill) {
						Div(w_fill, left_right)
						{
							// Heavy computation worker
							Div(mb(20), p(12), bg(45, 35, 35), rounded(6), w_fill) {
								Text("Heavy Computation Worker",
									fontSize(16), mb(10), textColour(255, 150, 150));

								if (heavyWorker.IsRunning()) {
									Div(left_right, mb(10)) {
										Rect(w(12), h(12), bg(255, 200, 0), rounded(6), mr(8), centerY);
										Text("Computing...", textColour(255, 200, 0), centerY, wrapText(false));
									}
								}
								else if (heavyWorker.IsCompleted()) {
									auto result = heavyWorker.GetResult();
									Div(mb(5), w_fill) {
										Text(SableString::Format("Result: %d", result.value),
											textColour(100, 255, 100));
									}
									Div(mb(5), w_fill) {
										Text(SableString::Format("Operation: %s", result.operation.c_str()),
											textColour(150, 150, 150));
									}
									Text(SableString::Format("Duration: %d ms", result.durationMs),
										textColour(150, 150, 150));
								}
								else if (heavyWorker.IsFailed()) {
									Text("Worker failed: " + heavyWorker.GetError(),
										textColour(255, 100, 100));
								}
								else {
									Text("Idle - Click 'Start Heavy Task' to begin",
										textColour(120, 120, 120));
								}

								Div(mt(10), left_right) {
									ButtonWithVariant("Start Heavy Task (2s)", [this]() {
										heavyWorker.Reset();
										heavyWorker.Start();
										}, ButtonVariant::Primary, w_fit);

									if (heavyWorker.IsRunning()) {
										Div(w(10), h(1));
										ButtonWithVariant("Cancel", [this]() {
											heavyWorker.Cancel();
											heavyWorker.Reset();
										}, ButtonVariant::Danger, w_fit);
									}
								}
							}

							// Light computation worker
							Div(mb(20), p(12), bg(35, 35, 45), rounded(6), w_fill) {
								Text("Light Computation Worker",
									fontSize(16), mb(10), textColour(150, 150, 255));

								if (lightWorker.IsRunning()) {
									Div(left_right, mb(10)) {
										Rect(w(12), h(12), bg(100, 200, 255), rounded(6), mr(8), centerY);
										Text("Computing...", textColour(100, 200, 255), centerY, wrapText(false));
									}
								}
								else if (lightWorker.IsCompleted()) {
									auto result = lightWorker.GetResult();
									Div(mb(5), w_fill) {
										Text(SableString::Format("Result: %d", result.value),
											textColour(100, 255, 100));
									}
									Div(mb(5), w_fill) {
										Text(SableString::Format("Operation: %s", result.operation.c_str()),
											textColour(150, 150, 150));
									}
									Text(SableString::Format("Duration: %d ms", result.durationMs),
										textColour(150, 150, 150));
								}
								else if (lightWorker.IsFailed()) {
									Text("Worker failed: " + lightWorker.GetError(),
										textColour(255, 100, 100));
								}
								else {
									Text("Idle - Click 'Start Light Task' to begin",
										textColour(120, 120, 120));
								}

								Div(mt(10), left_right) {
									ButtonWithVariant("Start Light Task (0.5s)", [this]() {
										lightWorker.Reset();
										lightWorker.Start();
										}, ButtonVariant::Primary, w_fit);

									if (lightWorker.IsRunning()) {
										Div(w(10), h(1));
										ButtonWithVariant("Cancel", [this]() {
											lightWorker.Cancel();
											lightWorker.Reset();
											}, ButtonVariant::Danger, w_fit);
									}
								}
							}

							// Stress test
							Div(p(12), bg(45, 45, 35), rounded(6)) {
								Text("Stress Test", fontSize(16), mb(10), textColour(255, 255, 150));
								Text(SableString::Format("Tasks completed: %d", stressTaskCount.get()),
									mb(10), textColour(200, 200, 200));

								ButtonWithVariant("Launch 10 Worker Tasks", [this]() {
									LaunchStressTasks();
									}, ButtonVariant::Danger, w_fit);
							}
						}
					}

					// === STATUS SECTION ===
					Div(p(15), bg(35, 35, 40), rounded(8), w_fill) {
						Text("System Status", fontSize(20), mb(15), textColour(255, 200, 100));

						Div(mb(8)) {
							Text(SableString::Format("Active Intervals: %d",
								(clockInterval.IsRunning() ? 1 : 0) + (blinkInterval.IsRunning() ? 1 : 0)),
								textColour(180, 180, 180), wrapText(false));
						}
						Div(mb(8)) {
							Text(SableString::Format("Active Workers: %d",
								(heavyWorker.IsRunning() ? 1 : 0) + (lightWorker.IsRunning() ? 1 : 0)),
								textColour(180, 180, 180), wrapText(false));
						}
						Div(mb(8)) {
							Text(SableString::Format("Pending Timeouts: %d",
								testTimeout.IsPending() ? 1 : 0),
								textColour(180, 180, 180), wrapText(false));
						}
					}
				}
			}
		}

		void OnUpdate(const UIEventContext& ctx) override
		{
			ScrollUpdateHandler(scrollCtx);
		}

		void OnUpdatePostLayout(const UIEventContext& ctx) override
		{
			ScrollUpdatePostLayoutHandler(scrollCtx);
		}

	private:
		ScrollContext scrollCtx;
		// Timer states
		State<SableString> currentTime{ this, "00:00:00" };
		State<int> uptime{ this, 0 };
		State<bool> blink{ this, false };
		State<SableString> timeoutMessage{ this, "" };
		State<int> stressTaskCount{ this, 0 };

		// Timers
		Interval clockInterval{ this, 1.0f, [this]() {
			UpdateTime();
			uptime.set(uptime.get() + 1);
		} };

		Interval blinkInterval{ this, 0.5f, [this]() {
			blink.set(!blink.get());
		} };

		Timeout testTimeout{ this };

		// Workers
		WorkerState<ComputeResult> heavyWorker{ this };
		WorkerState<ComputeResult> lightWorker{ this };

		int GetSimplePseudoRandom(int min, int max) {
			static unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
			seed = (1103515245 * seed + 12345);
			return min + (int)(seed % (max - min + 1));
		}

		void UpdateTime() {
			auto now = std::time(nullptr);
			auto tm = *std::localtime(&now);
			std::ostringstream oss;
			oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << tm.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << tm.tm_sec;
			currentTime.set(oss.str());
		}

		ComputeResult PerformHeavyComputation() {
			auto start = std::chrono::steady_clock::now();

			// Simulate heavy computation (2 seconds)
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));

			int result = GetSimplePseudoRandom(1000, 9999);

			auto end = std::chrono::steady_clock::now();
			int duration = (int)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

			return ComputeResult{ result, "Heavy: Prime computation", duration };
		}

		ComputeResult PerformLightComputation() {
			auto start = std::chrono::steady_clock::now();

			// Simulate light computation (500ms)
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			int result = GetSimplePseudoRandom(1, 100);

			auto end = std::chrono::steady_clock::now();
			int duration = (int)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

			return ComputeResult{ result, "Light: Quick calculation", duration };
		}

		void LaunchStressTasks() {
			for (int i = 0; i < 10; i++) {
				WorkerPool::Submit([this, i]() {
					std::this_thread::sleep_for(std::chrono::milliseconds(100 + (i * 50)));
					stressTaskCount.set(stressTaskCount.get() + 1);
					});
			}
		}
	};
}