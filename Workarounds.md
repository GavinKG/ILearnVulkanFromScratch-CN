Gitbook package does not support newer version of node.js. Here are some workarounds to get gitbook running without migrating whole blog to other platforms:

* set node version to: 10.23.0 (use nvm)

* use gitbook v2.3.2

* change ~/.gitbook/versions/lib/output/website/copyPluginAssets.js:112 to

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

  