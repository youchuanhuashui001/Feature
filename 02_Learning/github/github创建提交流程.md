# Github 使用

用于记录创建公共仓库，基础仓库如何 `clone、push` 代码使用。



前提：需要在 github 中提交开发公匙

- pc 生成公匙：

  ```
  生成公钥：ssh-keygen -t rsa -C "*@*.com"
  查看公钥：cat ~/.ssh/id_rsa.pub
  ```

- github 中将公匙保存到账户：

  ```
  github --> setting --> ssh and gpg keys --> new ssh key
  ```

  

## 公共/私有仓库

- github 中创建好新的仓库
- pc 中使用 `ssh` 的方式 `git clone`:`git clone git@github.com:youchuanhuashui001/download_bin.git`
- `git add`
- `git commit -m "xxx"`
- `git push`






