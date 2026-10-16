package com.facebook.react.bridge

// React markers do not own an Activity. This host does not start a JS runtime.
object ReactMarker {
  fun interface MarkerListener { fun logMarker(name: ReactMarkerConstants, tag: String?, instanceKey: Int) }
  private val listeners = mutableSetOf<MarkerListener>()
  @JvmStatic fun addListener(listener: MarkerListener) { listeners.add(listener) }
  @JvmStatic fun removeListener(listener: MarkerListener) { listeners.remove(listener) }
}
enum class ReactMarkerConstants { CONTENT_APPEARED }
