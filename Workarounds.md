Gitbook package does not support newer version of node.js. Here are some workarounds to get gitbook running without migrating whole blog to other platforms:

* set node version to: 10.23.0 (use nvm)

  * (optional) set nvm mirror to taobao:

    ```
    node_mirror: https://npm.taobao.org/mirrors/node/
    npm_mirror: https://npm.taobao.org/mirrors/npm/
    ```

* use gitbook v2.3.2

  ```shell
  npm install -g gitbook-cli@2.3.2
  ```

  when using gitbook command for the first time, it will install Gitbook 3.2.3.

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

  

Migrating to hexo is considered.

