classdef Csuperstate_mngr < handle
% Csuper_state_mngr calculate superstate of marker field
    
    methods (Static)
        
        function s_state = calc_super_state(pool,state)
            %mustBeA(pool,'Dpool');
            %mustBeA(state,'Dfield');
            
           %% update superstate based on the remaining tags in pool
            % "acc_p0"/"acc_p1" are the accumulated possibilities of the
            % elements being 0/1 (an element might be included by multiple tags from
            % different poses. The possibilities are independent with each other,
            % thus they are multipled.)
            % "acc0"/"acc1" are the supported numbers of the elements being 0/1 (an
            % element might be included by multiple tags. The supported number
            % is the minimum).
            
            [M,N] = size(state.val);
            acc_p0 = ones(M,N);    
            acc_p1 = ones(M,N);
            acc0 = Inf*ones(M,N);
            acc1 = Inf*ones(M,N);

            hit_map = Chit.hit_partially(state,pool);
            
            shape_num = size(pool.val,1);
            for t_shape = 1 : shape_num
                % Accumulate the possibilities based on the hit results.
                % There are two situations when a region is not hit by any tag:
                % 1. all the elements in this region is settled;
                % 2. some elements in this region can be neither 0 nor 1.
                type_num = size(pool.rot_prop{t_shape},1);
                for t_type = 1 : type_num
                    cur_rot = pool.rot_prop{t_shape}(t_type,1);
                    [inst_M,inst_N,~] = size(pool.val{t_shape,cur_rot});
                    for acc_m = 1 : size(hit_map.val{t_shape,cur_rot},1)
                        for acc_n = 1 : size(hit_map.val{t_shape,cur_rot},2)
                            count0 = 0;
                            count1 = 0;
                            flag_hollow = false;
                            for t_rot = 1 : size(pool.rot_prop{t_shape},2)
                                cur_rot = pool.rot_prop{t_shape}(t_type,t_rot);
                                if hit_map.val{t_shape,cur_rot}(acc_m,acc_n,1)==-2
                                    flag_hollow = true;
                                end
                                hit_inst = pool.val{t_shape,cur_rot}(:,:,hit_map.val{t_shape,cur_rot}(acc_m,acc_n,:)==1);
                                count0 = count0 + sum((hit_inst)==0,3);
                                count1 = count1 + sum((hit_inst)==1,3);
                            end

                            temp_p0 = count0./(count0+count1);
                            temp_p1 = count1./(count0+count1);

                            unconcerned = pool.val{t_shape,cur_rot}(:,:,1)==-1;
                            if flag_hollow
                                unconcerned = true(size(pool.val{t_shape,cur_rot}(:,:,1)));
                            end
                            count0(unconcerned) = Inf;
                            count1(unconcerned) = Inf;
                            temp_p0(unconcerned) = 1;
                            temp_p1(unconcerned) = 1;

                            m_range = acc_m:acc_m+inst_M-1;
                            n_range = acc_n:acc_n+inst_N-1;
                            acc_p0(m_range,n_range) = acc_p0(m_range,n_range) .* temp_p0;
                            acc_p1(m_range,n_range) = acc_p1(m_range,n_range) .* temp_p1;
                            acc0(m_range,n_range) = min(acc0(m_range,n_range),count0);
                            acc1(m_range,n_range) = min(acc1(m_range,n_range),count1);
                        end
                    end
                end
            end % for t = 1 : size(pool,1)

            likelihood = acc_p1./(acc_p0+acc_p1);
            support = min(acc0,acc1);
            
            likelihood(state.val>=0) = -1;
            support(state.val>=0) = -1;
            likelihood(state.val==-2) = -2;
            support(state.val==-2) = -2;
            stuck_pos = (isnan(likelihood) & (support)==0);
            likelihood(stuck_pos) = -3;
            support(stuck_pos) = -3;
            
            s_state = Dsuperstate(likelihood,support);
        end % function
        
    end % method
    
end

