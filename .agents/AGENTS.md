## Automatic Version Bump Rule

- **Automatic Increment**: Whenever making code or configuration changes for a request, automatically bump the patch version by `0.0.1` in both `package.json` and `package.template.json`.
- **User Prompt**: At the end of the response, state the newly auto-bumped version and ask the user if they would like to modify it before committing.
