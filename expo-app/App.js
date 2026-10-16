import React, { useEffect, useState } from 'react';
import { Button, SafeAreaView, Text } from 'react-native';
import { LinearGradient } from 'expo-linear-gradient';
import * as SplashScreen from 'expo-splash-screen';

SplashScreen.preventAutoHideAsync();
SplashScreen.setOptions({ duration: 400, fade: true });

export default function App() {
  const [blue, setBlue] = useState(false);
  useEffect(() => { SplashScreen.hide(); }, []);
  return (
    <SafeAreaView style={{ flex: 1, justifyContent: 'center', padding: 24, gap: 16 }}>
      <Text>Recreate or finish this Activity using the adb commands in README.md.</Text>
      <LinearGradient
        testID="gradient-surface"
        colors={blue ? ['#1e3a8a', '#0ea5e9'] : ['#b91c1c', '#fbbf24']}
        style={{ height: 180, borderRadius: 16 }}
      />
      <Text testID="gradient-palette">Palette: {blue ? 'blue' : 'red'}</Text>
      <Button testID="gradient-colors-toggle" title="Change gradient colors" onPress={() => setBlue((value) => !value)} />
      <Button testID="splash-set-options" title="Set splash options again" onPress={() => SplashScreen.setOptions({ duration: 400, fade: true })} />
    </SafeAreaView>
  );
}
