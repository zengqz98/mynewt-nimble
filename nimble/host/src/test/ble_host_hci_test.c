/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "nimble/hci_common.h"
#include "host/host_hci.h"
#include "host/ble_hs.h"
#include "host/ble_hs_test.h"
#include "testutil/testutil.h"
#include "ble_l2cap.h"
#include "ble_hs_conn.h"
#include "ble_hs_att.h"
#include "ble_hs_att_cmd.h"
#include "ble_hs_test_util.h"

TEST_CASE(ble_host_hci_test_event_bad)
{
    uint8_t buf[2];
    int rc;

    /*** Invalid event code. */
    buf[0] = 0xff;
    buf[1] = 0;
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == ENOTSUP);
}

TEST_CASE(ble_host_hci_test_event_cmd_complete)
{
    uint8_t buf[BLE_HCI_EVENT_CMD_COMPLETE_HDR_LEN];
    int rc;

    rc = ble_hs_init();
    TEST_ASSERT_FATAL(rc == 0);

    /*** Unsent OCF. */
    ble_hs_test_util_build_cmd_complete(buf, sizeof buf, 1, 1, 12345);
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == ENOENT);

    /*** No error on NOP. */
    ble_hs_test_util_build_cmd_complete(buf, sizeof buf, 1, 1,
                                              BLE_HCI_OPCODE_NOP);
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == 0);

    /*** Acknowledge sent command. */
    rc = host_hci_cmd_le_set_adv_enable(0);
    TEST_ASSERT(rc == 0);
    ble_hs_test_util_build_cmd_complete(
        buf, sizeof buf, 1, 1,
        (BLE_HCI_OGF_LE << 10) | BLE_HCI_OCF_LE_SET_ADV_ENABLE);
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == 0);

    /*** Duplicate ack is error. */
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == ENOENT);
}

TEST_CASE(ble_host_hci_test_event_cmd_status)
{
    uint8_t buf[BLE_HCI_EVENT_CMD_STATUS_LEN];
    int rc;

    rc = ble_hs_init();
    TEST_ASSERT_FATAL(rc == 0);

    /*** Unsent OCF. */
    ble_hs_test_util_build_cmd_status(buf, sizeof buf, 0, 1, 12345);
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == ENOENT);

    /*** No error on NOP. */
    ble_hs_test_util_build_cmd_complete(buf, sizeof buf, 0, 1,
                                              BLE_HCI_OPCODE_NOP);
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == 0);

    /*** Acknowledge sent command. */
    rc = host_hci_cmd_le_set_adv_enable(0);
    TEST_ASSERT(rc == 0);
    ble_hs_test_util_build_cmd_status(
        buf, sizeof buf, BLE_ERR_SUCCESS, 1,
        (BLE_HCI_OGF_LE << 10) | BLE_HCI_OCF_LE_SET_ADV_ENABLE);
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == 0);

    /*** Duplicate ack is error. */
    rc = host_hci_event_rx(buf);
    TEST_ASSERT(rc == ENOENT);
}

TEST_SUITE(ble_host_hci_suite)
{
    ble_host_hci_test_event_bad();
    ble_host_hci_test_event_cmd_complete();
    ble_host_hci_test_event_cmd_status();
}

int
ble_host_hci_test_all(void)
{
    ble_host_hci_suite();
    return tu_any_failed;
}
