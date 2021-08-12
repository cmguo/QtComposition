import os

from QtToolsFish import conans_tools
from QtToolsFish.Conans import QtConanFile

package_name = "QtComposition"
package_version = "master"

package_user_channel = "cmguo/stable"


class ConanConfig(QtConanFile):
    name = package_name
    version = package_version

    git_url = "git@github.com:cmguo/QtComposition.git"


if __name__ == '__main__':
    conans_tools.remove_cache(package_version=f"{package_name}/{package_version}", user_channel=package_user_channel)
    conans_tools.create(user_channel=package_user_channel)
    conans_tools.upload(package_version=f"{package_name}/{package_version}", user_channel=package_user_channel)
