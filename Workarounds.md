* node version: 10.23.0 (use nvm)

* gitbook version v2.3.2

* ~/.gitbook/versions/lib/output/website/copyPluginAssets.js:112

  ```js
  return fs.copyDir(
          assetsFolder,
          assetOutputFolder,
          {
              deleteFirst: false,
              overwrite: true,
              // Edited this line from true to false
              confirm: false
          }
      );
  ```

  