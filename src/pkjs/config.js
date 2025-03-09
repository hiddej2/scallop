module.exports = [
    {
      "type": "heading",
      "defaultValue": "Scallop Watchface"
    },
    {
      "type": "text",
      "defaultValue": "By hiddej2."
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Colors"
        },
        {
          "type": "color",
          "messageKey": "BackgroundColor",
          "defaultValue": "0x000000",
          "label": "Background Color"
        },
        {
          "type": "color",
          "messageKey": "ForegroundColor",
          "defaultValue": "0xFFFFFF",
          "label": "Foreground Color"
        },
        {
            "type": "color",
            "messageKey": "MinuteHand",
            "defaultValue": "0x000000",
            "label": "Minute Hand Color"
          },
          {
            "type": "color",
            "messageKey": "HourHand",
            "defaultValue": "0x000000",
            "label": "Hour Hand Color"
          }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Black and White settings"
        },
        {
          "type": "toggle",
          "messageKey": "Invert",
          "label": "Invert in case of B/W",
          "defaultValue": true
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
  ];