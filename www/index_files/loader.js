(function () {
  document.currentScript =
    document.currentScript ||
    (function () {
      var scripts = document.getElementsByTagName('script');
      return scripts[scripts.length - 1];
    })();
  var allAttrs = document.currentScript.attributes;
  var locationID = allAttrs['data-location-id'];
  var cdnURL = allAttrs['data-cdn-url'] && allAttrs['data-cdn-url'].value;
  if (!allAttrs['data-resources-url']) {
    var indexOfLastPath = document.currentScript.src.lastIndexOf('/');
    var sourceURL = document.currentScript.src.substring(0, indexOfLastPath);
    document.currentScript.setAttribute('data-resources-url', sourceURL + '/chat-widget/loader.js');
  }

  if (!cdnURL) {
    if (document.currentScript.src) {
      var lastPathIndex = document.currentScript.src.lastIndexOf('/');
      cdnURL = document.currentScript.src.substring(0, lastPathIndex);
    } else {
      cdnURL = window.location.origin;
    }
  }

  function loadWidget() {
    var moduleScript = document.createElement('script');
    moduleScript.type = 'module';
    moduleScript.src = cdnURL + '/chat-widget/chat-widget.esm.js';

    var plainScript = document.createElement('script');
    plainScript.noModule = true;

    plainScript.src = cdnURL + '/chat-widget/chat-widget.js';
    plainScript.setAttribute('data-resources-url', cdnURL + '/chat-widget/chat-widget.js');
    plainScript.setAttribute('data-stencil-namespace', cdnURL + 'chat-widget');
    document.head.appendChild(moduleScript);
    document.head.appendChild(plainScript);

    var gCaptchScript = document.createElement('script');
    gCaptchScript.src = 'https://www.google.com/recaptcha/api.js?render=explicit';
    gCaptchScript.setAttribute('defer', '');
    gCaptchScript.setAttribute('async', '');
    document.head.appendChild(gCaptchScript);

    var nonCustomElement = document.querySelector('div[data-chat-widget]');
    var chatWidgetElement = document.getElementsByTagName('chat-widget');

    if (nonCustomElement !== null && chatWidgetElement.length === 0) {
      allAttrs = nonCustomElement.attributes;
      locationID = allAttrs['data-location-id'];
      try {
        if (!!nonCustomElement.remove) nonCustomElement.remove();
        else nonCustomElement.parentNode.removeChild(nonCustomElement);
      } catch (error) {}
    }
    if (!!locationID) {
      var textWidget = document.createElement('chat-widget');

      for (var attrIndex = allAttrs.length - 1; attrIndex >= 0; attrIndex--) {
        var attributeName = allAttrs[attrIndex].name;
        var attributeValue = allAttrs[attrIndex].value;
        try {
          attributeName = attributeName.replace('data-', '');
        } catch (e) {
          console.log(e, 'Fail to load widget');
        }
        if (attributeName === 'style') {
          attributeValue =
            attributeValue +
            `;
        --message-outgoing-text-color: #fff !important;
        --message-incoming-color: #e4e9f0;
        --message-incoming-text-color: #333;
        --conversation-background-color: #fff;
        --message-check-mark-color: #399bf0;
        --message-padding-content: 1em;
        --message-padding-between: 0;
        --message-round-size: 0.5em;
        --message-round-size-negative: calc(-1 * var(--message-round-size));`;
        }
        textWidget.setAttribute(attributeName, attributeValue);
      }

      document.body.appendChild(textWidget);
    }
    if (chatWidgetElement.length > 0 && !locationID) {
      for (var i = 0; i < chatWidgetElement.length; i++) {
        var styleAttribute = chatWidgetElement[i].getAttribute('style');
        if (styleAttribute) {
          styleAttribute =
            styleAttribute +
            `;
        --message-outgoing-text-color: #fff !important;
        --message-incoming-color: #e4e9f0;
        --message-incoming-text-color: #333;
        --conversation-background-color: #fff;
        --message-check-mark-color: #399bf0;
        --message-padding-content: 1em;
        --message-padding-between: 0;
        --message-round-size: 0.5em;
        --message-round-size-negative: calc(-1 * var(--message-round-size));`;

          chatWidgetElement[i].setAttribute('style', styleAttribute);
        }
      }
    }
  }

  if (document.readyState !== 'loading') {
    setTimeout(function () {
      loadWidget();
    }, 0);
  } else {
    //somehow window.addEventListener not working with rocket loader
    document.addEventListener('DOMContentLoaded', function () {
      setTimeout(function () {
        loadWidget();
      }, 0);
    });
  }
})();
