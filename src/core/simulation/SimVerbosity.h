#pragma once

// Global verbosity flag for simulation core logs (Bootstrap, StepPlanner, etc.).
// Default: true (logs to stderr).
// Set to false for headless/test runs where only TelemetryRecorder output is wanted.
//
// Thread-safety: single-threaded simulation only. Set once before the run loop.
extern bool g_sim_verbose;
