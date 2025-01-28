# CONTRIBUTING

For code formatting and style, see the [Style guide](docs/styleguide.md).

## Development process

`CFGrid` development is centred around the [GitHub
repository](https://github.com/CFLaboratory/CFGrid) with planned work outlined in issues.

Before beginning work on a feature, please check the issues and pull requests in case there is
similar work that is either already in progress or planned - you may be able to contribute here.
If there are no such issues/pull requests, please open an issue to describe the proposed
contribution so that we can ensure this fits with the plans for `CFGrid` development and potential
issues can be addressed before beginning the effort.

The `CFGrid` repository is based on two primary branches: `develop` which is used as the base of new
developments, and `main` which new releases are deployed to.
New developments should be made in feature branches starting from `develop`.
To contribute your work back to `CFGrid` open a pull request against the `develop` branch to begin
the review process, it is encouraged that pull requests are opened early in the development cycle
for awareness.

To maintain clarity of the development history developers are encouraged to `squash` intermediate
commits on their branch before it is merged so that the primary changes are clear; feature branches
should `rebase` against the `develop` branch regularly to ensure the code in development remains up
to date.
Commit messages should be clear with informative titles following [Conventional
Commits](https://www.conventionalcommits.org/en/v1.0.0/), and either during or at the end of
development of a feature branch the [CHANGELOG](CHANGELOG.md) should be updated to outline the
changes made.
