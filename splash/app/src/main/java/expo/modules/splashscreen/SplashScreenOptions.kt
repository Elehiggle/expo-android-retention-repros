package expo.modules.splashscreen

// The native manager reads duration only. No Expo record conversion runs in this host.
class SplashScreenOptions {
  val duration: Long = 400L
  var fade: Boolean = true
}
