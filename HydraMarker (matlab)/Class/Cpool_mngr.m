classdef Cpool_mngr < handle
% Cpool_mngr offers functions to manipulate tag pools
    
    methods (Static)
        
        function [msg,sub_pool] = remove_exist(pool,state)
        % remove the tags that already appear in state matrix
        % the remaining tags are returned in sub_pool
        % msg might be "success" or "error_one_to_many_hit"
            %mustBeA(pool,'Dpool');
            %mustBeA(state,'Dfield');
            sub_pool = pool;
            
            hit_map = Chit.hit_fully(state,pool);
            for t = 1 : size(hit_map.val,1)
                hit_time = zeros(size(hit_map.val{t,1},3),4);
                hit_map.val{t,1}(hit_map.val{t,1}<0) = NaN;
                hit_map.val{t,2}(hit_map.val{t,2}<0) = NaN;
                hit_map.val{t,3}(hit_map.val{t,3}<0) = NaN;
                hit_map.val{t,4}(hit_map.val{t,4}<0) = NaN;
                hit_time(:,1) = permute(sum(hit_map.val{t,1},[1,2],'omitnan'),[3,1,2]);
                hit_time(:,2) = permute(sum(hit_map.val{t,2},[1,2],'omitnan'),[3,1,2]);
                hit_time(:,3) = permute(sum(hit_map.val{t,3},[1,2],'omitnan'),[3,1,2]);
                hit_time(:,4) = permute(sum(hit_map.val{t,4},[1,2],'omitnan'),[3,1,2]);
                hit_time = sum(hit_time,2);

                if any(hit_time(:)>1)
                    % the uniqueness are violated if a tag hits more than one position
                    msg = Dmsg('tag_repeated');
                    return;
                end                
                % removed the tags that fully hit the marker field
                % the tags in the same rows (their rotated versions) are removed too
                sub_pool.val{t,1}(:,:,hit_time>0) = [];
                sub_pool.val{t,2}(:,:,hit_time>0) = [];
                sub_pool.val{t,3}(:,:,hit_time>0) = [];
                sub_pool.val{t,4}(:,:,hit_time>0) = [];
                
                assert(~isempty(sub_pool.val{t,1}),'tag pool is exhausted!');
            end % end for 
            
            msg = Dmsg('empty');
        end % end function
        
    end % end method
    
end

