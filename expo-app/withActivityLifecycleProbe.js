const { withMainActivity } = require('expo/config-plugins');

module.exports = (config) => withMainActivity(config, (mod) => {
  if (mod.modResults.language !== 'kt') throw new Error('This reproduction needs the Kotlin activity template');
  if (mod.modResults.contents.includes('org.example.splashretentionexpo.RECREATE')) return mod;
  if (mod.modResults.contents.includes('override fun onNewIntent')) throw new Error('Review the generated onNewIntent before adding the lifecycle probe');
  const method = `
  override fun onNewIntent(intent: android.content.Intent) {
    when (intent.action) {
      "org.example.splashretentionexpo.RECREATE" -> recreate()
      "org.example.splashretentionexpo.FINISH" -> finish()
      else -> super.onNewIntent(intent)
    }
  }
`;
  if (!/\n}\s*$/.test(mod.modResults.contents)) throw new Error('MainActivity template has changed');
  mod.modResults.contents = mod.modResults.contents.replace(/\n}\s*$/, `${method}\n}\n`);
  return mod;
});
