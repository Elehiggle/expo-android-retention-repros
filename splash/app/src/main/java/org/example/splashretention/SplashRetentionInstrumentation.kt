package org.example.splashretention

import android.app.Activity
import android.app.Instrumentation
import android.content.Intent
import android.os.Bundle
import android.os.SystemClock
import expo.modules.splashscreen.SplashScreenManager
import expo.modules.splashscreen.SplashScreenOptions
import java.lang.ref.WeakReference

class SplashRetentionInstrumentation : Instrumentation() {
  @Volatile private var phase = "created"
  private var launchCount = 0

  private fun progress(message: String) {
    phase = message
    sendStatus(1, Bundle().apply { putString("phase", message) })
  }

  override fun onCreate(arguments: Bundle?) {
    super.onCreate(arguments)
    start()
  }

  override fun onStart() {
    val results = Bundle()
    val runnerThread = Thread.currentThread()
    val deadline = Thread({
      try {
        Thread.sleep(60_000)
        finish(Activity.RESULT_CANCELED, Bundle().apply {
          putString("result", "FAIL")
          putString("failure", "Test timed out after 60 seconds at: $phase")
          putString("runner_stack", runnerThread.stackTrace.joinToString("\n"))
        })
      } catch (_: InterruptedException) {
      }
    }, "SplashReproDeadline").apply { isDaemon = true; start() }
    try {
      progress("headless options")
      runOnMainSync { SplashScreenManager.setSplashScreenOptions(SplashScreenOptions()) }
      results.putString("headless_options", "PASS")
      progress("older/newer owner check")
      checkNewerOwner()
      results.putString("newer_owner", "PASS")
      progress("destroyed owner and Activity check")
      val destroyed = finishActivity()
      val ownerReleased = owner() == null
      results.putString("destroyed_owner", if (ownerReleased) "PASS" else "FAIL")
      repeat(20) {
        if (destroyed.get() != null) {
          Runtime.getRuntime().gc()
          System.runFinalization()
          SystemClock.sleep(100)
          waitForIdleSync()
        }
      }
      val activityReleased = destroyed.get() == null
      results.putString("destroyed_activity", if (activityReleased) "PASS" else "FAIL")
      check(ownerReleased && activityReleased) { "Destroyed owner released=$ownerReleased; destroyed Activity collected=$activityReleased" }
      runOnMainSync { SplashScreenManager.setSplashScreenOptions(SplashScreenOptions()) }
      results.putString("options_after_destroy", "PASS")
      progress("all checks passed")
      results.putString("result", "PASS")
      finish(Activity.RESULT_OK, results)
    } catch (failure: Throwable) {
      results.putString("result", "FAIL")
      results.putString("failure", failure.stackTraceToString())
      finish(Activity.RESULT_CANCELED, results)
    } finally {
      deadline.interrupt()
    }
  }

  private fun launch(): Activity {
    launchCount += 1
    progress("start Activity $launchCount")
    val activity = startActivitySync(
      Intent(targetContext, SplashActivity::class.java)
        .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
    )
    progress("Activity $launchCount started; wait for idle")
    waitForIdleSync()
    progress("Activity $launchCount idle")
    return activity
  }

  private fun owner(): Any? {
    var value: Any? = null
    runOnMainSync {
      val field = SplashScreenManager::class.java.getDeclaredField("splashScreen")
      field.isAccessible = true
      value = field.get(null)
    }
    return value
  }

  private fun checkNewerOwner() {
    val older = launch()
    val oldOwner = owner()
    val newer = launch()
    val newOwner = owner()
    check(oldOwner != null && newOwner != null && oldOwner !== newOwner)
    finishAndWait(older)
    check(owner() === newOwner) { "Destroying an older Activity cleared the newer splash screen" }
    finishAndWait(newer)
  }

  private fun finishAndWait(activity: Activity) {
    runOnMainSync { activity.finish() }
    repeat(100) {
      var destroyed = false
      runOnMainSync { destroyed = activity.isDestroyed }
      if (destroyed) {
        waitForIdleSync()
        return
      }
      SystemClock.sleep(50)
    }
    error("Activity did not reach onDestroy within five seconds")
  }

  private fun finishActivity(): WeakReference<Activity> {
    val activity = launch()
    val weak = WeakReference(activity)
    finishAndWait(activity)
    return weak
  }
}
