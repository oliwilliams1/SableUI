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
			const Theme& t = GetTheme();

			ScrollViewCtx(scrollCtx, w_fill, h_fill, bg(t.base))
			{
				Div(w_fill, h_fill, p(20)) {
					Text("Worker Pool & Timer System Test",
						fontSize(28), mb(20), textColour(t.text));

					// === TIMER SECTION ===
					Div(mb(30), p(15), bg(t.surface0), rounded(8), w_fill) {
						Text("Timer System", fontSize(20), mb(15), textColour(t.sky));

						Div(mb(15), left_right) {
							Div(w_fit) {
								Text("Current Time:", mb(5), textColour(t.subtext0));
								Text(currentTime.get(), fontSize(24), textColour(t.sky));
							}
							Div(w(20), h(1));
							Div(w_fit) {
								Text("Uptime:", mb(5), textColour(t.subtext0));
								Text(SableString::Format("%d seconds", uptime.get()),
									fontSize(24), textColour(t.peach));
							}
						}

						Div(left_right, mb(15)) {
							Colour blinkColour = (blink.get()) ? t.green : t.surface2;
							Rect(w(16), h(16),
								bg(blinkColour),
								rounded(8), mr(10), centerY);
							Text("Interval (500ms) - Blink Status",
								textColour(t.subtext1), centerY, wrapText(false));
						}

						if (!timeoutMessage.get().empty()) {
							Div(p(10), bg(t.mauve), rounded(5), mb(10)) {
								Text(timeoutMessage.get(), textColour(t.text));
							}
						}

						Div(left_right, h_fit) {
							Button("Reset Uptime", [this]() {
								uptime.set(0);
								}, bg(t.secondary), mr(4));

							Button(clockInterval.IsRunning() ? "Pause Clock" : "Resume Clock",
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
								bg(clockInterval.IsRunning() ? t.red : t.primary), mr(4));

							Button("Trigger 3s Timeout", [this]() {
								timeoutMessage.set("Timeout scheduled...");
								testTimeout.Schedule(3.0f, [this]() {
									timeoutMessage.set("Timeout fired after 3 seconds!");
									});
								}, bg(t.primary));
						}
					}

					// === WORKER SECTION ===
					Div(mb(30), p(15), bg(t.surface0), rounded(8), w_fill) {
						Text("Worker System", fontSize(20), mb(15), textColour(t.sky));

						// Heavy computation worker
						Div(mb(20), p(12), bg(t.surface1), rounded(6), w_fill, mr(8)) {
							Text("Heavy Computation Worker",
								fontSize(16), mb(10), textColour(t.red));

							if (heavyWorker.IsRunning()) {
								Div(left_right, mb(10)) {
									Rect(w(12), h(12), bg(t.yellow), rounded(6), mr(8), centerY);
									Text("Computing...", textColour(t.yellow), centerY, wrapText(false));
								}
							}
							else if (heavyWorker.IsCompleted()) {
								auto result = heavyWorker.GetResult();
								Div(mb(5), w_fill) {
									Text(SableString::Format("Result: %d", result.value),
										textColour(t.green));
								}
								Div(mb(5), w_fill) {
									Text(SableString::Format("Operation: %s", result.operation.c_str()),
										textColour(t.subtext1));
								}
								Text(SableString::Format("Duration: %d ms", result.durationMs),
									textColour(t.subtext1));
							}
							else if (heavyWorker.IsFailed()) {
								Text("Worker failed: " + heavyWorker.GetError(),
									textColour(t.red));
							}
							else {
								Text("Idle - Click 'Start Heavy Task' to begin",
									textColour(t.subtext0));
							}

							Div(mt(10), left_right, h_fit) {
								Button("Start Heavy Task (2s)", [this]() {
									heavyWorker.Reset();
									heavyWorker.Start();
									}, bg(t.primary), mr(4));

								if (heavyWorker.IsRunning()) {
									Button("Cancel", [this]() {
										heavyWorker.Cancel();
										heavyWorker.Reset();
										}, bg(t.red));
								}
							}
						}

						// Light computation worker
						Div(mb(20), p(12), bg(t.surface1), rounded(6), w_fill) {
							Text("Light Computation Worker",
								fontSize(16), mb(10), textColour(t.blue));

							if (lightWorker.IsRunning()) {
								Div(left_right, mb(10)) {
									Rect(w(12), h(12), bg(t.sky), rounded(6), mr(8), centerY);
									Text("Computing...", textColour(t.sky), centerY, wrapText(false));
								}
							}
							else if (lightWorker.IsCompleted()) {
								auto result = lightWorker.GetResult();
								Div(mb(5), w_fill) {
									Text(SableString::Format("Result: %d", result.value),
										textColour(t.green));
								}
								Div(mb(5), w_fill) {
									Text(SableString::Format("Operation: %s", result.operation.c_str()),
										textColour(t.subtext1));
								}
								Text(SableString::Format("Duration: %d ms", result.durationMs),
									textColour(t.subtext1));
							}
							else if (lightWorker.IsFailed()) {
								Text("Worker failed: " + lightWorker.GetError(),
									textColour(t.red));
							}
							else {
								Text("Idle - Click 'Start Light Task' to begin",
									textColour(t.subtext0));
							}

							Div(mt(10), left_right, h_fit) {
								Button("Start Light Task (0.5s)", [this]() {
									lightWorker.Reset();
									lightWorker.Start();
									}, bg(t.primary), mr(4));

								if (lightWorker.IsRunning()) {
									Button("Cancel", [this]() {
										lightWorker.Cancel();
										lightWorker.Reset();
										}, bg(t.red));
								}
							}
						}

						// Stress test
						Div(p(12), bg(t.surface1), rounded(6), w_fill) {
							Text("Stress Test", fontSize(16), mb(10), textColour(t.yellow));
							Text(SableString::Format("Tasks completed: %d", stressTaskCount.get()),
								mb(10), textColour(t.text));

							Button("Launch 10 Worker Tasks", [this]() {
								LaunchStressTasks();
								}, bg(t.red), w_fit);
						}
					}

					// === STATUS SECTION ===
					Div(p(15), bg(t.surface0), rounded(8), w_fill) {
						Text("System Status", fontSize(20), mb(15), textColour(t.peach));

						Div(mb(8)) {
							Text(SableString::Format("Active Intervals: %d",
								(clockInterval.IsRunning() ? 1 : 0) + (blinkInterval.IsRunning() ? 1 : 0)),
								textColour(t.subtext1), wrapText(false));
						}
						Div(mb(8)) {
							Text(SableString::Format("Active Workers: %d",
								(heavyWorker.IsRunning() ? 1 : 0) + (lightWorker.IsRunning() ? 1 : 0)),
								textColour(t.subtext1), wrapText(false));
						}
						Div(mb(8)) {
							Text(SableString::Format("Pending Timeouts: %d",
								testTimeout.IsPending() ? 1 : 0),
								textColour(t.subtext1), wrapText(false));
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