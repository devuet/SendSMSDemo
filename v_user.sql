/*
MySQL Backup
Source Server Version: 5.7.21
Source Database: ludeng
Date: 2019/1/16 17:21:54
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
--  View definition for `v_user`
-- ----------------------------
DROP VIEW IF EXISTS `v_user`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_user` AS select `t_personnel`.`user_id` AS `user_id`,`t_personnel`.`id` AS `id`,`t_person`.`unit_id` AS `unit_id`,`t_person`.`telephone` AS `telephone`,`t_unit`.`district_id` AS `district_id` from ((`t_personnel` join `t_person`) join `t_unit`) where ((`t_person`.`id` = `t_personnel`.`id`) and (`t_unit`.`unit_id` = `t_person`.`unit_id`));

-- ----------------------------
--  Records 
-- ----------------------------
