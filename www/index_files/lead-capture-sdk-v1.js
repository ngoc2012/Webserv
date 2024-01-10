const loadScriptAsync = (uri) => {
  return new Promise((resolve, reject) => {
    let tag = document.createElement('script');
    tag.src = uri;
    tag.setAttribute('async', 'true');
    tag.onload = () => {
      resolve();
    };
    let firstScriptTag = document.getElementsByTagName('script')[0];
    firstScriptTag.parentNode.insertBefore(tag, firstScriptTag);
  });
};
const ldLoaded = loadScriptAsync('https://cdn.jsdelivr.net/npm/launchdarkly-js-client-sdk@3');

const injectScripts = (version) => {
  const lcaiScript = document.createElement('script');
  lcaiScript.type = 'text/javascript';
  if (version === 'local') {
    lcaiScript.src = '/lead-capture-sdk.local.js';
  } else {
    lcaiScript.src = `https://leads.aurorasolar.com/lead-capture-sdk.${version || 'next'}.js`;
  }
  lcaiScript.setAttribute('async', 'true');

  document.body.appendChild(lcaiScript);
};

const handleError = () => {
  injectScripts();
};

/*
Order of precedence for obtaining the LCAI Version
- script tag data-version attribute
- launchdarkly version from 'v2Version' variation
- fallback version to next
*/
const auroraScript = document.getElementById('auroraLCAI');

if (auroraScript && auroraScript.hasAttribute('data-version')) {
  injectScripts(auroraScript.getAttribute('data-version'));
} else {
  try {
    ldLoaded
      .then(() => {
        const ldClient = LDClient.initialize('5f511b5803298309572edfd1', {
          anonymous: true,
          custom: {
            host: 'autoroof.auslr.io',
          },
        });
        ldClient
          .waitUntilReady()
          .then(() => {
            const ldVersion = ldClient.variation('v2Version');
            injectScripts(ldVersion);
          })
          .catch(handleError);
      })
      .catch(handleError);
  } catch (error) {
    handleError();
  }
}
