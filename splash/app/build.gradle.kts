plugins { id("com.android.application") }
android {
  namespace = "org.example.splashretention"
  compileSdk = 36
  defaultConfig {
    applicationId = "org.example.splashretention"
    minSdk = 26
    targetSdk = 36
    versionCode = 1
    versionName = "1.0"
  }
  buildFeatures { buildConfig = true }
}
dependencies { implementation("androidx.core:core-splashscreen:1.2.0") }
