UPDATE `instance_spawn_groups` SET `bossStates`= 23 WHERE `bossStates`= 55;

DELETE FROM `spawn_group_template` WHERE `groupId`= 403;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(403, 'The Vortex Pinnacle - Grandvizier Ertan Trash', 4);

SET @SPAWNGROUP := 403;
DELETE FROM `spawn_group` WHERE `groupId`= @SPAWNGROUP;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(@SPAWNGROUP, 0, 368186),
(@SPAWNGROUP, 0, 368180),
(@SPAWNGROUP, 0, 368197),
(@SPAWNGROUP, 0, 368187),
(@SPAWNGROUP, 0, 368179),
(@SPAWNGROUP, 0, 368198),
(@SPAWNGROUP, 0, 368199),
(@SPAWNGROUP, 0, 368200),
(@SPAWNGROUP, 0, 368190),
(@SPAWNGROUP, 0, 368189),
(@SPAWNGROUP, 0, 368188),
(@SPAWNGROUP, 0, 368181),
(@SPAWNGROUP, 0, 368183),
(@SPAWNGROUP, 0, 368185),
(@SPAWNGROUP, 0, 368182),
(@SPAWNGROUP, 0, 368192),
(@SPAWNGROUP, 0, 368191),
(@SPAWNGROUP, 0, 368184),
(@SPAWNGROUP, 0, 368194),
(@SPAWNGROUP, 0, 368193),
(@SPAWNGROUP, 0, 368201),
(@SPAWNGROUP, 0, 368203),
(@SPAWNGROUP, 0, 368195),
(@SPAWNGROUP, 0, 368196),
(@SPAWNGROUP, 0, 368205),
(@SPAWNGROUP, 0, 368202),
(@SPAWNGROUP, 0, 368204);

DELETE FROM `spawn_group_template` WHERE `groupId`= 404;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(404, 'The Vortex Pinnacle - Altairus Trash', 4);

SET @SPAWNGROUP := 404;
DELETE FROM `spawn_group` WHERE `groupId`= @SPAWNGROUP;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(@SPAWNGROUP, 0, 368223),
(@SPAWNGROUP, 0, 368230),
(@SPAWNGROUP, 0, 368222),
(@SPAWNGROUP, 0, 368221),
(@SPAWNGROUP, 0, 368220),
(@SPAWNGROUP, 0, 368224),
(@SPAWNGROUP, 0, 368225),
(@SPAWNGROUP, 0, 368226),
(@SPAWNGROUP, 0, 368227),
(@SPAWNGROUP, 0, 368242),
(@SPAWNGROUP, 0, 368235),
(@SPAWNGROUP, 0, 368234),
(@SPAWNGROUP, 0, 368233),
(@SPAWNGROUP, 0, 368232),
(@SPAWNGROUP, 0, 368244),
(@SPAWNGROUP, 0, 368245),
(@SPAWNGROUP, 0, 368238),
(@SPAWNGROUP, 0, 368239),
(@SPAWNGROUP, 0, 368236),
(@SPAWNGROUP, 0, 368237),
(@SPAWNGROUP, 0, 368243),
(@SPAWNGROUP, 0, 368240),
(@SPAWNGROUP, 0, 368228),
(@SPAWNGROUP, 0, 368229),
(@SPAWNGROUP, 0, 368231),
(@SPAWNGROUP, 0, 368241);

DELETE FROM `spawn_group_template` WHERE `groupId`= 405;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(405, 'The Vortex Pinnacle - Asaad Trash', 4);

SET @SPAWNGROUP := 405;
DELETE FROM `spawn_group` WHERE `groupId`= @SPAWNGROUP;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(@SPAWNGROUP, 0, 368263),
(@SPAWNGROUP, 0, 368260),
(@SPAWNGROUP, 0, 368269),
(@SPAWNGROUP, 0, 368277),
(@SPAWNGROUP, 0, 368264),
(@SPAWNGROUP, 0, 368271),
(@SPAWNGROUP, 0, 368272),
(@SPAWNGROUP, 0, 368265),
(@SPAWNGROUP, 0, 368278),
(@SPAWNGROUP, 0, 368295),
(@SPAWNGROUP, 0, 368279),
(@SPAWNGROUP, 0, 368281),
(@SPAWNGROUP, 0, 368284),
(@SPAWNGROUP, 0, 368286),
(@SPAWNGROUP, 0, 368282),
(@SPAWNGROUP, 0, 368283),
(@SPAWNGROUP, 0, 368285),
(@SPAWNGROUP, 0, 368280),
(@SPAWNGROUP, 0, 368270),
(@SPAWNGROUP, 0, 368267),
(@SPAWNGROUP, 0, 368261),
(@SPAWNGROUP, 0, 368266),
(@SPAWNGROUP, 0, 368273),
(@SPAWNGROUP, 0, 368276),
(@SPAWNGROUP, 0, 368262),
(@SPAWNGROUP, 0, 368268),
(@SPAWNGROUP, 0, 368275),
(@SPAWNGROUP, 0, 368274),
(@SPAWNGROUP, 0, 368296),
(@SPAWNGROUP, 0, 368289),
(@SPAWNGROUP, 0, 368288),
(@SPAWNGROUP, 0, 368292),
(@SPAWNGROUP, 0, 368291),
(@SPAWNGROUP, 0, 368293),
(@SPAWNGROUP, 0, 368294),
(@SPAWNGROUP, 0, 368290),
(@SPAWNGROUP, 0, 368287);

DELETE FROM `instance_spawn_groups` WHERE `instanceMapId`= 657;
INSERT INTO `instance_spawn_groups` (`instanceMapId`, `bossStateId`, `bossStates`, `spawnGroupId`, `flags`) VALUES
(657, 0, 17, 403, 1), -- Enable group when Grandvizier Ertan is not DONE
(657, 1, 17, 404, 1), -- Enable group when Alairus is not DONE
(657, 2, 17, 405, 1); -- Enable group when Asaad is not DONE