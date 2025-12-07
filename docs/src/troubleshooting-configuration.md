# Troubleshooting
**Build fails with "cannot find OpenGL" or simmilar** (Linux)
```bash
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```
^^ Or replace with your package manager
> There may be additional libraries, if so this page will be updated later

**CMake can't find dependencies** Check that you cloned with the --recursive flag. If not:
```bash
git submodule update --init --recursive
```
For more undocumented issues, you can [open an issue](https://github.com/oliwilliams1/SableUI/issues).

Now you have configured SableUI, you can now create [your first application](your-first-application.md)!