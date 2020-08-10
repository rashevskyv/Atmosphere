
#pragma once
#include "impl/fspusb_usb_manager.hpp"
#include "fspusb_filesystem.hpp"
#include <stratosphere/fssrv/fssrv_interface_adapters.hpp>

using IFileSystemInterface = ams::fssrv::impl::FileSystemInterfaceAdapter;

namespace ams::mitm::fspusb {

    class Service : public sf::IServiceObject {
        private:
            enum class CommandId {
                ListMountedDrives = 0,
                GetDriveFileSystemType = 1,
                GetDriveLabel = 2,
                SetDriveLabel = 3,
                OpenDriveFileSystem = 4,
            };

        public:
            void ListMountedDrives(const sf::OutArray<s32> &out_interface_ids, sf::Out<s32> out_count);
            Result GetDriveFileSystemType(s32 drive_interface_id, sf::Out<u8> out_fs_type);
            Result GetDriveLabel(s32 drive_interface_id, sf::OutBuffer &out_label_str);
            Result SetDriveLabel(s32 drive_interface_id, sf::InBuffer &label_str);
            Result OpenDriveFileSystem(s32 drive_interface_id, sf::Out<std::shared_ptr<IFileSystemInterface>> out_fs);

            DEFINE_SERVICE_DISPATCH_TABLE {
                MAKE_SERVICE_COMMAND_META(ListMountedDrives),
                MAKE_SERVICE_COMMAND_META(GetDriveFileSystemType),
                MAKE_SERVICE_COMMAND_META(GetDriveLabel),
                MAKE_SERVICE_COMMAND_META(SetDriveLabel),
                MAKE_SERVICE_COMMAND_META(OpenDriveFileSystem),
            };
    };

}